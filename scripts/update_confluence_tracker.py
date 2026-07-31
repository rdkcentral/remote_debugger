#!/usr/bin/env python3
"""
Appends a row to a branch-specific Confluence tracker table every time this
repo gets a push to main/develop/release branches (commit-level row) or
publishes a release (release-level row).

Required environment variables (typically populated in workflow env from
GitHub Secrets):
    TRACKER_CONFLUENCE_BASE_URL   e.g. https://yourorg.atlassian.net/wiki
    TRACKER_CONFLUENCE_EMAIL      Atlassian account email tied to the API token
    TRACKER_CONFLUENCE_API_TOKEN  https://id.atlassian.com/manage-profile/security/api-tokens
    TRACKER_CONFLUENCE_PAGE_ID    numeric ID of the target Confluence page

Supplied automatically by the workflow (from GitHub context values):
    TRACKER_COMPONENT_NAME, TRACKER_EVENT_NAME, TRACKER_GITHUB_SHA,
    TRACKER_GITHUB_REF_NAME, TRACKER_GITHUB_REPOSITORY,
    TRACKER_GITHUB_SERVER_URL, TRACKER_ACTOR,
    TRACKER_RELEASE_TAG, TRACKER_RELEASE_NAME,
    TRACKER_RELEASE_BODY, TRACKER_COMMIT_MESSAGE
"""

import html
import os
import re
import sys
from datetime import datetime, timezone

import requests
from bs4 import BeautifulSoup

TABLE_HEADERS = [
    "Date (UTC)",
    "Component",
    "Type",
    "Version / Commit",
    "Author",
    "Ticket(s)",
    "Release Fix Status",
    "Details",
    "Summary",
]
SUMMARY_TABLE_HEADERS = ["Date (UTC)", "Component", "Summary"]
MAX_ROWS = 200  # keep the page from growing unbounded; oldest rows drop off
MAIN_TABLE_TITLE = "Main Branch Updates"
DEVELOP_TABLE_TITLE = "Develop Branch Updates"
RELEASE_BRANCH_TABLE_TITLE = "Release Branch Updates"
SUPPORT_BRANCH_TABLE_TITLE = "Support Branch Updates"
PUBLISHED_RELEASES_TABLE_TITLE = "GitHub Component Releases"
LEGACY_PUBLISHED_RELEASES_TABLE_TITLE = "Published Releases"
RELEASE_TABLE_TITLE_ALIASES = [LEGACY_PUBLISHED_RELEASES_TABLE_TITLE, PUBLISHED_RELEASES_TABLE_TITLE]
L1_OUTPUT_SUMMARY_TITLE = "L1 Output Summary"
L2_OUTPUT_SUMMARY_TITLE = "L2 Output Summary"
TRACKER_TABLE_TITLES = {
    MAIN_TABLE_TITLE,
    DEVELOP_TABLE_TITLE,
    RELEASE_BRANCH_TABLE_TITLE,
    SUPPORT_BRANCH_TABLE_TITLE,
    PUBLISHED_RELEASES_TABLE_TITLE,
    LEGACY_PUBLISHED_RELEASES_TABLE_TITLE,
}

# Preferred variable names used by this script.
# Each key supports a legacy fallback env var for backward compatibility.
ENV_ALIASES = {
    "CONFLUENCE_BASE_URL": ("TRACKER_CONFLUENCE_BASE_URL", "CONFLUENCE_BASE_URL"),
    "CONFLUENCE_EMAIL": ("TRACKER_CONFLUENCE_EMAIL", "CONFLUENCE_EMAIL"),
    "CONFLUENCE_API_TOKEN": ("TRACKER_CONFLUENCE_API_TOKEN", "CONFLUENCE_API_TOKEN"),
    "CONFLUENCE_PAGE_ID": ("TRACKER_CONFLUENCE_PAGE_ID", "CONFLUENCE_PAGE_ID"),
    "COMPONENT_NAME": ("TRACKER_COMPONENT_NAME", "COMPONENT_NAME"),
    "EVENT_NAME": ("TRACKER_EVENT_NAME", "EVENT_NAME"),
    "GITHUB_SHA": ("TRACKER_GITHUB_SHA", "GITHUB_SHA"),
    "GITHUB_REF_NAME": ("TRACKER_GITHUB_REF_NAME", "GITHUB_REF_NAME"),
    "GITHUB_REPOSITORY": ("TRACKER_GITHUB_REPOSITORY", "GITHUB_REPOSITORY"),
    "GITHUB_SERVER_URL": ("TRACKER_GITHUB_SERVER_URL", "GITHUB_SERVER_URL"),
    "ACTOR": ("TRACKER_ACTOR", "ACTOR"),
    "RELEASE_TAG": ("TRACKER_RELEASE_TAG", "RELEASE_TAG"),
    "RELEASE_NAME": ("TRACKER_RELEASE_NAME", "RELEASE_NAME"),
    "RELEASE_BODY": ("TRACKER_RELEASE_BODY", "RELEASE_BODY"),
    "COMMIT_MESSAGE": ("TRACKER_COMMIT_MESSAGE", "COMMIT_MESSAGE"),
    "JIRA_BASE_URL": ("TRACKER_JIRA_BASE_URL", "JIRA_BASE_URL"),
    "ENABLE_JIRA_LINKS": ("TRACKER_ENABLE_JIRA_LINKS", "ENABLE_JIRA_LINKS"),
    "RELEASE_FIX_STATUS": ("TRACKER_RELEASE_FIX_STATUS", "RELEASE_FIX_STATUS"),
    "L1_SUMMARY": ("TRACKER_L1_SUMMARY", "L1_SUMMARY"),
    "L2_SUMMARY": ("TRACKER_L2_SUMMARY", "L2_SUMMARY"),
}

TICKET_PATTERN = re.compile(r"\b([A-Z][A-Z0-9]+-\d+)\b")


def env(name, default=""):
    return os.environ.get(name, default)


def cfg(key, default=""):
    """Resolve a logical config key from preferred variable names/fallbacks."""
    for name in ENV_ALIASES.get(key, (key,)):
        value = env(name)
        if value:
            return value
    return default


def resolved_env_name(key):
    """Return which env var name satisfied a logical key, if any."""
    for name in ENV_ALIASES.get(key, (key,)):
        if env(name):
            return name
    return None


def log_env_resolution():
    """Log only env variable names used for config resolution (no values)."""
    tracked_keys = [
        "CONFLUENCE_BASE_URL",
        "CONFLUENCE_EMAIL",
        "CONFLUENCE_API_TOKEN",
        "CONFLUENCE_PAGE_ID",
        "COMPONENT_NAME",
        "EVENT_NAME",
        "GITHUB_SHA",
        "GITHUB_REF_NAME",
        "GITHUB_REPOSITORY",
        "GITHUB_SERVER_URL",
        "ACTOR",
        "RELEASE_TAG",
        "RELEASE_NAME",
        "RELEASE_BODY",
        "COMMIT_MESSAGE",
        "JIRA_BASE_URL",
        "RELEASE_FIX_STATUS",
        "L1_SUMMARY",
        "L2_SUMMARY",
    ]

    print("Config env resolution (name only):")
    for key in tracked_keys:
        name = resolved_env_name(key)
        print(f"  {key}: {name if name else 'MISSING'}")


def build_row_data():
    """Turn the GitHub event context into a single tracker row."""
    event_name = cfg("EVENT_NAME")
    repo = cfg("GITHUB_REPOSITORY")
    server = cfg("GITHUB_SERVER_URL")
    jira_base_url = cfg("JIRA_BASE_URL").rstrip("/")
    enable_jira_links = cfg("ENABLE_JIRA_LINKS", "false").strip().lower() in {"1", "true", "yes", "on"}
    effective_jira_base_url = jira_base_url if enable_jira_links else ""
    component = cfg("COMPONENT_NAME", repo.split("/")[-1] if repo else "unknown")
    now = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")
    l1_summary = cfg("L1_SUMMARY").strip()
    l2_summary = cfg("L2_SUMMARY").strip()

    def extract_tickets(text):
        if not text:
            return []
        seen = set()
        ordered = []
        for match in TICKET_PATTERN.findall(text.upper()):
            if match not in seen:
                seen.add(match)
                ordered.append(match)
        return ordered

    if event_name == "release":
        tag = cfg("RELEASE_TAG")
        name = cfg("RELEASE_NAME") or tag
        release_body = cfg("RELEASE_BODY").strip()
        body_first_line = release_body.splitlines()[0] if release_body else ""
        link = f"{server}/{repo}/releases/tag/{tag}"
        return {
            "date": now,
            "component": component,
            "type": "Release",
            "version": tag,
            "author": cfg("ACTOR"),
            "tickets": ", ".join(extract_tickets(f"{name}\n{release_body}")) or "-",
            "jira_base_url": effective_jira_base_url,
            "release_fix_status": "-",
            "details": name,
            "changeset": release_body,
            "details_summary": body_first_line,
            "l1_summary": l1_summary,
            "l2_summary": l2_summary,
            "link": link,
        }
    else:
        sha_full = cfg("GITHUB_SHA")
        sha = sha_full[:7]
        commit_message = cfg("COMMIT_MESSAGE").strip()
        msg = commit_message.splitlines()[0] if commit_message else "(no message)"
        tickets = extract_tickets(commit_message)
        link = f"{server}/{repo}/commit/{sha_full}"
        label = "Manual sync" if event_name == "workflow_dispatch" else "Commit"
        release_fix_status = cfg("RELEASE_FIX_STATUS").strip() or "Not evaluated"
        return {
            "date": now,
            "component": component,
            "type": label,
            "version": sha,
            "author": cfg("ACTOR"),
            "tickets": ", ".join(tickets) or "-",
            "jira_base_url": effective_jira_base_url,
            "release_fix_status": release_fix_status,
            "details": msg[:200],
            "l1_summary": l1_summary,
            "l2_summary": l2_summary,
            "link": link,
        }


def table_title_for_event():
    """Select which Confluence table this event should update."""
    event_name = cfg("EVENT_NAME")
    ref_name = cfg("GITHUB_REF_NAME")

    if event_name == "release":
        return PUBLISHED_RELEASES_TABLE_TITLE
    if ref_name == "main":
        return MAIN_TABLE_TITLE
    if ref_name == "develop":
        return DEVELOP_TABLE_TITLE
    if ref_name == "release" or ref_name.startswith("release/"):
        return RELEASE_BRANCH_TABLE_TITLE
    if ref_name == "support" or ref_name.startswith("support/"):
        return SUPPORT_BRANCH_TABLE_TITLE
    # Fallback keeps unexpected refs from being dropped.
    return f"Branch Updates: {ref_name or 'unknown'}"


class ConfluenceClient:
    def __init__(self, base_url, email, token):
        self.base_url = base_url.rstrip("/")
        self.auth = (email, token)
        self.session = requests.Session()
        self.session.auth = self.auth
        self.session.headers.update({"Content-Type": "application/json"})

    def get_page(self, page_id):
        url = f"{self.base_url}/api/v2/pages/{page_id}"
        resp = self.session.get(url, params={"body-format": "storage"})
        resp.raise_for_status()
        return resp.json()

    def update_page(self, page_id, title, new_storage_value, new_version):
        url = f"{self.base_url}/api/v2/pages/{page_id}"
        payload = {
            "id": str(page_id),
            "status": "current",
            "title": title,
            "body": {"representation": "storage", "value": new_storage_value},
            "version": {"number": new_version, "message": "Automated tracker update"},
        }
        resp = self.session.put(url, json=payload)
        resp.raise_for_status()
        return resp.json()


def make_row_html(row):
    def cell(text):
        return f"<td><p>{html.escape(str(text))}</p></td>"

    def tickets_cell_html(tickets_text, jira_base_url):
        tickets_text = (tickets_text or "-").strip()
        if tickets_text in {"", "-"}:
            return "<td><p>-</p></td>"

        tickets = [t.strip() for t in tickets_text.split(",") if t.strip()]
        if not tickets:
            return "<td><p>-</p></td>"

        if not jira_base_url:
            return f"<td><p>{html.escape(', '.join(tickets))}</p></td>"

        parts = []
        for ticket in tickets:
            href = f"{jira_base_url}/browse/{ticket}"
            parts.append(f'<a href="{html.escape(href)}">{html.escape(ticket)}</a>')
        return f"<td><p>{', '.join(parts)}</p></td>"

    def expand_block(title, inner_html):
        if not inner_html:
            return ""
        return (
            '<ac:structured-macro ac:name="expand">'
            f'<ac:parameter ac:name="title">{html.escape(title)}</ac:parameter>'
            f"<ac:rich-text-body>{inner_html}</ac:rich-text-body>"
            "</ac:structured-macro>"
        )

    def changeset_expand(changeset_text):
        lines = [line for line in (changeset_text or "").splitlines() if line.strip()]
        if not lines:
            return ""
        body_html = "".join(f"<p>{html.escape(line)}</p>" for line in lines)
        return (
            '<ac:structured-macro ac:name="expand">'
            '<ac:parameter ac:name="title">Changeset</ac:parameter>'
            f"<ac:rich-text-body>{body_html}</ac:rich-text-body>"
            "</ac:structured-macro>"
        )

    def summary_expand(title, text):
        summary_text = (text or "").strip()
        if not summary_text:
            return ""
        body_html = "".join(f"<p>{html.escape(line)}</p>" for line in summary_text.splitlines() if line.strip())
        return expand_block(title, body_html)

    details_text = html.escape(str(row.get("details", "")))
    summary = row.get("details_summary", "")
    summary_html = f"<p>{html.escape(str(summary))}</p>" if summary else ""
    changeset_html = changeset_expand(row.get("changeset", ""))
    details_cell = f"<td>{summary_html}<p>{details_text}</p>{changeset_html}</td>"

    link_html = f'<p><a href="{html.escape(row["link"])}">{html.escape(row["type"])} link</a></p>'
    links_expand_html = expand_block("Links", link_html)

    l1_text = (row.get("l1_summary", "") or "").strip()
    l2_text = (row.get("l2_summary", "") or "").strip()
    ai_parts = []
    if l1_text:
        ai_parts.append(f"<p><strong>L1:</strong> {html.escape(l1_text)}</p>")
    if l2_text:
        for line in l2_text.splitlines():
            if line.strip():
                ai_parts.append(f"<p><strong>L2:</strong> {html.escape(line)}</p>")
    ai_expand_html = expand_block("AI Summary", "".join(ai_parts))

    summary_cell = f"<td>{links_expand_html}{ai_expand_html}</td>"

    return (
        "<tr>"
        + cell(row["date"])
        + cell(row["component"])
        + cell(row["type"])
        + cell(row["version"])
        + cell(row["author"])
        + tickets_cell_html(row.get("tickets", "-"), row.get("jira_base_url", ""))
        + cell(row.get("release_fix_status", "-"))
        + details_cell
        + summary_cell
        + "</tr>"
    )


def build_empty_table(headers=None):
    headers = headers or TABLE_HEADERS
    header_cells = "".join(f"<th><p><strong>{html.escape(h)}</strong></p></th>" for h in headers)
    return f'<table data-layout="default"><tbody><tr>{header_cells}</tr></tbody></table>'


def build_heading_html(title):
    return f"<h2><strong>{html.escape(title)}</strong></h2>"


def table_after_heading(heading):
    node = heading.next_sibling
    while node and getattr(node, "name", None) is None:
        node = node.next_sibling
    if node and node.name == "table":
        return node
    return None


def find_table_for_title(soup, title):
    for heading in soup.find_all(["h1", "h2", "h3", "h4"]):
        if heading.get_text(" ", strip=True) == title:
            table = table_after_heading(heading)
            if table is not None:
                return table
    return None


def find_table_for_any_title(soup, titles):
    for title in titles:
        table = find_table_for_title(soup, title)
        if table is not None:
            return table
    return None


def find_heading_and_table_for_titles(soup, titles):
    """Return (heading, table) for the first matching titled section."""
    for title in titles:
        for heading in soup.find_all(["h1", "h2", "h3", "h4"]):
            if heading.get_text(" ", strip=True) == title:
                table = table_after_heading(heading)
                if table is not None:
                    return heading, table
    return None, None


def normalize_table_columns(table, headers):
    """Ensure header labels and row cell counts match expected headers."""
    headers = headers or TABLE_HEADERS
    tbody = table.find("tbody") or table
    rows = tbody.find_all("tr")

    if rows:
        header_row = rows[0]
        header_row.clear()
    else:
        header_row = soup = BeautifulSoup("<tr></tr>", "html.parser").find("tr")
        tbody.insert(0, header_row)

    for header in headers:
        th = BeautifulSoup(
            f"<th><p><strong>{html.escape(header)}</strong></p></th>", "html.parser"
        ).find("th")
        header_row.append(th)

    expected_cells = len(headers)
    for row in tbody.find_all("tr")[1:]:
        cells = row.find_all(["td", "th"], recursive=False)
        while len(cells) < expected_cells:
            row.append(BeautifulSoup("<td><p></p></td>", "html.parser").find("td"))
            cells = row.find_all(["td", "th"], recursive=False)
        while len(cells) > expected_cells:
            cells[-1].decompose()
            cells = row.find_all(["td", "th"], recursive=False)


def move_section_to_top(soup, heading, table):
    """Move a heading+table block to the top of the page."""
    if heading is None or table is None:
        return
    heading.extract()
    table.extract()
    soup.insert(0, table)
    soup.insert(0, heading)


def ensure_table_for_title(soup, table_title, headers=None):
    if table_title == PUBLISHED_RELEASES_TABLE_TITLE:
        heading, table = find_heading_and_table_for_titles(soup, RELEASE_TABLE_TITLE_ALIASES)
        if table is not None:
            # Keep release information as the first section for visibility.
            move_section_to_top(soup, heading, table)
            normalize_table_columns(table, headers or TABLE_HEADERS)
            return table

    table = find_table_for_title(soup, table_title)
    if table is not None:
        normalize_table_columns(table, headers or TABLE_HEADERS)
        return table

    # Backward compatibility: treat an existing unlabelled first table as main.
    if table_title == MAIN_TABLE_TITLE and not any(find_table_for_title(soup, title) for title in TRACKER_TABLE_TITLES):
        existing_table = soup.find("table")
        if existing_table is not None:
            normalize_table_columns(existing_table, headers or TABLE_HEADERS)
            return existing_table

    heading = BeautifulSoup(build_heading_html(table_title), "html.parser").find("h2")
    table = BeautifulSoup(build_empty_table(headers), "html.parser").find("table")

    if table_title == PUBLISHED_RELEASES_TABLE_TITLE:
        soup.insert(0, table)
        soup.insert(0, heading)
    else:
        soup.append(heading)
        soup.append(table)

    normalize_table_columns(table, headers or TABLE_HEADERS)

    return table


def ensure_output_summary_sections(storage_html):
    """Create placeholder L1/L2 output summary sections if missing."""
    soup = BeautifulSoup(storage_html or "", "html.parser")
    ensure_table_for_title(soup, L1_OUTPUT_SUMMARY_TITLE, SUMMARY_TABLE_HEADERS)
    ensure_table_for_title(soup, L2_OUTPUT_SUMMARY_TITLE, SUMMARY_TABLE_HEADERS)
    return str(soup)


def build_l1_summary_row():
    """Build L1 summary row.

    TODO: Replace placeholder return with your L1 data pull + transformation.
    Return a dict with keys: date, component, summary.
    Return None to skip update for this run.
    """
    summary = cfg("L1_SUMMARY").strip()
    if not summary:
        return None

    now = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")
    component = cfg("COMPONENT_NAME", cfg("GITHUB_REPOSITORY").split("/")[-1] if cfg("GITHUB_REPOSITORY") else "unknown")
    return {
        "date": now,
        "component": component,
        "summary": summary[:500],
    }


def build_l2_summary_row():
    """Build L2 summary row.

    TODO: Replace placeholder return with your L2 data pull + transformation.
    Return a dict with keys: date, component, summary.
    Return None to skip update for this run.
    """
    summary = cfg("L2_SUMMARY").strip()
    if not summary:
        return None

    now = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")
    component = cfg("COMPONENT_NAME", cfg("GITHUB_REPOSITORY").split("/")[-1] if cfg("GITHUB_REPOSITORY") else "unknown")
    return {
        "date": now,
        "component": component,
        "summary": summary[:2000],
    }


def make_summary_row_html(row):
    def cell(text):
        return f"<td><p>{html.escape(str(text))}</p></td>"

    return "<tr>" + cell(row["date"]) + cell(row["component"]) + cell(row["summary"]) + "</tr>"


def insert_summary_row(storage_html, row, table_title):
    """Insert a summary row into the selected L1/L2 summary table."""
    if not row:
        return storage_html

    soup = BeautifulSoup(storage_html or "", "html.parser")
    table = ensure_table_for_title(soup, table_title, SUMMARY_TABLE_HEADERS)

    tbody = table.find("tbody") or table
    header_row = tbody.find("tr")

    new_row_soup = BeautifulSoup(make_summary_row_html(row), "html.parser").find("tr")
    if header_row:
        header_row.insert_after(new_row_soup)
    else:
        tbody.append(new_row_soup)

    rows = tbody.find_all("tr")
    data_rows = rows[1:] if header_row else rows
    if len(data_rows) > MAX_ROWS:
        for extra in data_rows[MAX_ROWS:]:
            extra.decompose()

    return str(soup)


def insert_row(storage_html, row, table_title):
    """Insert the new row right under the header row of the selected table.
    If that table does not exist yet, create it at the end of the page."""
    soup = BeautifulSoup(storage_html or "", "html.parser")
    table = ensure_table_for_title(soup, table_title)

    tbody = table.find("tbody") or table
    header_row = tbody.find("tr")

    new_row_soup = BeautifulSoup(make_row_html(row), "html.parser").find("tr")
    if header_row:
        header_row.insert_after(new_row_soup)
    else:
        tbody.append(new_row_soup)

    # trim oldest rows beyond MAX_ROWS (keep header)
    rows = tbody.find_all("tr")
    if header_row:
        data_rows = rows[1:]
    else:
        data_rows = rows
    if len(data_rows) > MAX_ROWS:
        for extra in data_rows[MAX_ROWS:]:
            extra.decompose()

    return str(soup)


def main():
    log_env_resolution()

    base_url = cfg("CONFLUENCE_BASE_URL")
    email = cfg("CONFLUENCE_EMAIL")
    token = cfg("CONFLUENCE_API_TOKEN")
    page_id = cfg("CONFLUENCE_PAGE_ID")

    missing = [n for n, v in [
        ("CONFLUENCE_BASE_URL", base_url),
        ("CONFLUENCE_EMAIL", email),
        ("CONFLUENCE_API_TOKEN", token),
        ("CONFLUENCE_PAGE_ID", page_id),
    ] if not v]
    if missing:
        print(f"ERROR: missing required secrets/env vars: {', '.join(missing)}", file=sys.stderr)
        sys.exit(1)

    row = build_row_data()
    table_title = table_title_for_event()
    print(f"Prepared row for table '{table_title}': {row}")

    client = ConfluenceClient(base_url, email, token)
    page = client.get_page(page_id)

    title = page["title"]
    current_version = page["version"]["number"]
    current_body = page.get("body", {}).get("storage", {}).get("value", "")

    updated_body = insert_row(current_body, row, table_title)
    updated_body = ensure_output_summary_sections(updated_body)
    updated_body = insert_summary_row(updated_body, build_l1_summary_row(), L1_OUTPUT_SUMMARY_TITLE)
    updated_body = insert_summary_row(updated_body, build_l2_summary_row(), L2_OUTPUT_SUMMARY_TITLE)

    client.update_page(page_id, title, updated_body, current_version + 1)
    print(f"Confluence page '{title}' (ID {page_id}) updated to version {current_version + 1}.")


if __name__ == "__main__":
    main()
