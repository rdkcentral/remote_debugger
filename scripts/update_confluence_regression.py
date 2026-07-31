#!/usr/bin/env python3
"""Advanced regression tracker sync with containment KPI, alerts, trends and audit."""

import os
import statistics
import sys
from datetime import datetime, timezone

from bs4 import BeautifulSoup
from confluence_client import (
    ConfluenceClient, cell, link_cell, get_or_create_table,
)
from jira_client import build_jql, fetch_issues

HEADERS = ["Regression Ticket", "Summary", "Status", "Assignee", "Parent Ticket(s)", "Age (days)", "Containment SLA", "Updated", "Link"]
ALERT_HEADERS = ["Severity", "Alert", "Details", "Detected At"]
KPI_HEADERS = ["Date (UTC)", "Total Regressions", "Missing Parent", "Parent SLA Breaches", "Median Age (days)", "Containment KPI"]
TREND_HEADERS = ["Date (UTC)", "Total", "Missing Parent", "SLA Breaches", "New Regressions", "Resolved/Removed"]
ANOMALY_HEADERS = ["Issue", "Type", "Reason", "Detected At"]
AUDIT_HEADERS = ["Date (UTC)", "Job", "Fetched", "Inserted", "Updated", "Removed From Scope", "Missing Parent", "Result"]

ENV_ALIASES = {
    "CONFLUENCE_BASE_URL": ("TRACKER_CONFLUENCE_BASE_URL", "CONFLUENCE_BASE_URL"),
    "CONFLUENCE_EMAIL": ("TRACKER_CONFLUENCE_EMAIL", "CONFLUENCE_EMAIL"),
    "CONFLUENCE_API_TOKEN": ("TRACKER_CONFLUENCE_API_TOKEN", "CONFLUENCE_API_TOKEN"),
    "CONFLUENCE_REGRESSION_PAGE_ID": ("TRACKER_CONFLUENCE_REGRESSION_PAGE_ID", "CONFLUENCE_REGRESSION_PAGE_ID"),
    "JIRA_BASE_URL": ("TRACKER_JIRA_BASE_URL", "JIRA_BASE_URL"),
    "JIRA_EMAIL": ("TRACKER_JIRA_EMAIL", "JIRA_EMAIL"),
    "JIRA_API_TOKEN": ("TRACKER_JIRA_API_TOKEN", "JIRA_API_TOKEN"),
    "JIRA_REGRESSION_JQL": ("TRACKER_JIRA_REGRESSION_JQL", "JIRA_REGRESSION_JQL"),
    "JIRA_PARENT_LINK_TYPE": ("TRACKER_JIRA_PARENT_LINK_TYPE", "JIRA_PARENT_LINK_TYPE"),
    "JIRA_TEAM_NAME": ("TRACKER_JIRA_TEAM_NAME", "JIRA_TEAM_NAME"),
    "JIRA_TEAM_FIELD": ("TRACKER_JIRA_TEAM_FIELD", "JIRA_TEAM_FIELD"),
    "PARENT_LINK_SLA_DAYS": ("TRACKER_PARENT_LINK_SLA_DAYS", "PARENT_LINK_SLA_DAYS"),
    "KEEP_SNAPSHOT_ROWS": ("TRACKER_KEEP_SNAPSHOT_ROWS", "KEEP_SNAPSHOT_ROWS"),
    "REMOVE_STALE_ROWS": ("TRACKER_REMOVE_STALE_ROWS", "REMOVE_STALE_ROWS"),
}


def env(name, default=""):
    return os.environ.get(name, default)


def cfg(key, default=""):
    for name in ENV_ALIASES.get(key, (key,)):
        value = env(name)
        if value:
            return value
    return default


def as_int(value, default):
    try:
        return int(str(value).strip())
    except Exception:
        return default


def now_utc_text():
    return datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")


def parse_jira_datetime(value):
    if not value:
        return None
    value = value.strip()
    for fmt in ("%Y-%m-%dT%H:%M:%S.%f%z", "%Y-%m-%dT%H:%M:%S%z"):
        try:
            return datetime.strptime(value, fmt)
        except ValueError:
            continue
    return None


def extract_parents(issue, link_type_name):
    parents = []
    for link in issue["fields"].get("issuelinks", []):
        if link.get("type", {}).get("name", "").lower() != link_type_name.lower():
            continue
        linked_issue = link.get("outwardIssue") or link.get("inwardIssue")
        if linked_issue:
            parents.append(linked_issue["key"])
    return parents


def issue_to_row(issue, jira_base_url, link_type_name):
    fields = issue["fields"]
    assignee = fields.get("assignee")
    parents = extract_parents(issue, link_type_name)
    parent_display = ", ".join(parents) if parents else "⚠ No parent link found"
    created_dt = parse_jira_datetime(fields.get("created")) or parse_jira_datetime(fields.get("updated"))
    age_days = max(0, (datetime.now(timezone.utc) - created_dt.astimezone(timezone.utc)).days) if created_dt else 0
    parent_sla_days = as_int(cfg("PARENT_LINK_SLA_DAYS", "2"), 2)
    parent_sla = "OK"
    parent_sla_breach = False
    if not parents and age_days > parent_sla_days:
        parent_sla = f"BREACH (> {parent_sla_days}d without parent)"
        parent_sla_breach = True

    return {
        "key": issue["key"],
        "summary": (fields.get("summary") or "")[:150],
        "status": fields.get("status", {}).get("name", ""),
        "assignee": assignee["displayName"] if assignee else "Unassigned",
        "parents": parent_display,
        "age_days": age_days,
        "parent_sla": parent_sla,
        "parent_sla_breach": parent_sla_breach,
        "updated": (fields.get("updated") or "")[:16].replace("T", " "),
        "link": f"{jira_base_url.rstrip('/')}/browse/{issue['key']}",
    }


def make_row_html(row):
    return (
        "<tr>"
        + cell(row["key"]) + cell(row["summary"]) + cell(row["status"])
        + cell(row["assignee"]) + cell(row["parents"]) + cell(row["age_days"]) + cell(row["parent_sla"]) + cell(row["updated"])
        + link_cell(row["link"], "Jira")
        + "</tr>"
    )


def make_generic_row_html(values):
    return "<tr>" + "".join(cell(v) for v in values) + "</tr>"


def upsert_rows(soup, rows_by_key, remove_stale=False):
    table = get_or_create_table(soup, HEADERS, table_index=0)
    tbody = table.find("tbody") or table
    existing_rows = tbody.find_all("tr")
    header_row = existing_rows[0] if existing_rows else None
    data_rows = existing_rows[1:] if header_row else existing_rows

    existing_keys = []
    seen_keys = set()
    for tr in data_rows:
        first_cell = tr.find(["td", "th"])
        key = first_cell.get_text(strip=True) if first_cell else ""
        if key:
            existing_keys.append(key)
        if key in rows_by_key:
            new_tr = BeautifulSoup(make_row_html(rows_by_key[key]), "html.parser").find("tr")
            tr.replace_with(new_tr)
            seen_keys.add(key)

    inserted = 0
    insert_after = header_row if header_row else None
    for key, row in rows_by_key.items():
        if key in seen_keys:
            continue
        new_tr = BeautifulSoup(make_row_html(row), "html.parser").find("tr")
        if insert_after:
            insert_after.insert_after(new_tr)
            insert_after = new_tr
        else:
            tbody.append(new_tr)
        inserted += 1

    removed_scope = [k for k in existing_keys if k not in rows_by_key]
    if remove_stale and removed_scope:
        for tr in (tbody.find_all("tr")[1:] if tbody.find_all("tr") else []):
            first = tr.find(["td", "th"])
            key = first.get_text(strip=True) if first else ""
            if key in removed_scope:
                tr.decompose()

    return {"inserted": inserted, "updated": len(seen_keys), "removed_scope": removed_scope}


def replace_table_rows(soup, table_index, headers, rows):
    table = get_or_create_table(soup, headers, table_index=table_index)
    tbody = table.find("tbody") or table
    tbody.clear()
    header_html = "<tr>" + "".join(f"<th><p><strong>{h}</strong></p></th>" for h in headers) + "</tr>"
    tbody.append(BeautifulSoup(header_html, "html.parser").find("tr"))
    for row in rows:
        tbody.append(BeautifulSoup(make_generic_row_html(row), "html.parser").find("tr"))


def append_snapshot_row(soup, table_index, headers, row_values, keep_rows):
    table = get_or_create_table(soup, headers, table_index=table_index)
    tbody = table.find("tbody") or table
    if not tbody.find_all("tr"):
        header_html = "<tr>" + "".join(f"<th><p><strong>{h}</strong></p></th>" for h in headers) + "</tr>"
        tbody.append(BeautifulSoup(header_html, "html.parser").find("tr"))
    header_row = (tbody.find_all("tr") or [None])[0]
    header_row.insert_after(BeautifulSoup(make_generic_row_html(row_values), "html.parser").find("tr"))
    data_rows = (tbody.find_all("tr")[1:]) if tbody.find_all("tr") else []
    if len(data_rows) > keep_rows:
        for extra in data_rows[keep_rows:]:
            extra.decompose()


def main():
    base_url, email, token = cfg("CONFLUENCE_BASE_URL"), cfg("CONFLUENCE_EMAIL"), cfg("CONFLUENCE_API_TOKEN")
    page_id = cfg("CONFLUENCE_REGRESSION_PAGE_ID")
    jira_base_url, jira_email, jira_token = cfg("JIRA_BASE_URL"), cfg("JIRA_EMAIL"), cfg("JIRA_API_TOKEN")
    regression_jql = cfg("JIRA_REGRESSION_JQL")
    link_type_name = cfg("JIRA_PARENT_LINK_TYPE")
    team_name = cfg("JIRA_TEAM_NAME")
    team_field = cfg("JIRA_TEAM_FIELD", '"Team[Team]"')

    missing = [n for n, v in [
        ("CONFLUENCE_BASE_URL", base_url), ("CONFLUENCE_EMAIL", email),
        ("CONFLUENCE_API_TOKEN", token), ("CONFLUENCE_REGRESSION_PAGE_ID", page_id),
        ("JIRA_BASE_URL", jira_base_url), ("JIRA_EMAIL", jira_email),
        ("JIRA_API_TOKEN", jira_token), ("JIRA_REGRESSION_JQL", regression_jql),
        ("JIRA_PARENT_LINK_TYPE", link_type_name),
    ] if not v]
    if missing:
        print(f"ERROR: missing env vars: {', '.join(missing)}", file=sys.stderr)
        sys.exit(1)

    jql = build_jql(regression_jql, team_name, team_field)
    issues = fetch_issues(
        jira_base_url, jira_email, jira_token, jql,
        fields=["summary", "status", "assignee", "updated", "created", "issuelinks"],
    )
    print(f"Fetched {len(issues)} regression issues for JQL: {jql}")

    rows_by_key = {i["key"]: issue_to_row(i, jira_base_url, link_type_name) for i in issues}
    unresolved = [k for k, r in rows_by_key.items() if "No parent link found" in r["parents"]]
    if unresolved:
        print(f"WARNING: {len(unresolved)} regression ticket(s) missing a "
              f"'{link_type_name}' link to a parent: {', '.join(unresolved)}")

    client = ConfluenceClient(base_url, email, token)
    page = client.get_page(page_id)
    title = page["title"]
    version = page["version"]["number"]
    body = page.get("body", {}).get("storage", {}).get("value", "")
    soup = BeautifulSoup(body or "", "html.parser")

    keep_rows = as_int(cfg("KEEP_SNAPSHOT_ROWS", "120"), 120)
    remove_stale = cfg("REMOVE_STALE_ROWS", "false").strip().lower() in {"1", "true", "yes", "on"}
    stats = upsert_rows(soup, rows_by_key, remove_stale=remove_stale)

    now = now_utc_text()
    missing_parent = [k for k, r in rows_by_key.items() if "No parent link found" in r["parents"]]
    breaches = [k for k, r in rows_by_key.items() if r.get("parent_sla_breach")]
    alerts = [["high", "Missing parent link", f"{k} has no '{link_type_name}' parent link", now] for k in missing_parent]
    alerts += [["high", "Containment SLA breach", f"{k} exceeded parent-link SLA", now] for k in breaches]
    replace_table_rows(soup, 1, ALERT_HEADERS, alerts)

    ages = [r["age_days"] for r in rows_by_key.values()]
    median_age = round(statistics.median(ages), 1) if ages else 0
    containment_kpi = max(0.0, min(100.0, round(100 - (len(missing_parent) * 10) - (len(breaches) * 5), 1)))
    append_snapshot_row(soup, 2, KPI_HEADERS, [now, len(rows_by_key), len(missing_parent), len(breaches), median_age, containment_kpi], keep_rows)

    append_snapshot_row(soup, 3, TREND_HEADERS, [now, len(rows_by_key), len(missing_parent), len(breaches), stats["inserted"], len(stats["removed_scope"])], keep_rows)

    anomalies = []
    for key, row in rows_by_key.items():
        if row["assignee"] == "Unassigned":
            anomalies.append([key, "Data Quality", "Assignee is missing", now])
        if "No parent link found" in row["parents"]:
            anomalies.append([key, "Relationship", f"No '{link_type_name}' parent link", now])
    replace_table_rows(soup, 4, ANOMALY_HEADERS, anomalies)

    append_snapshot_row(soup, 5, AUDIT_HEADERS, [now, "jira-regression-to-confluence", len(rows_by_key), stats["inserted"], stats["updated"], len(stats["removed_scope"]), len(missing_parent), "success"], keep_rows)

    updated_body = str(soup)
    if updated_body == body:
        print("No changes to Confluence page content — skipping update.")
        return

    client.update_page_with_retry(page_id, title, updated_body, version + 1, message="Regression tracker sync")
    print(f"Confluence page '{title}' updated with regression KPIs.")


if __name__ == "__main__":
    main()
