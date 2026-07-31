#!/usr/bin/env python3
"""Advanced Jira delivery sync with SLA, alerts, health, drift and audit outputs."""

import os
import sys
from collections import defaultdict
from datetime import datetime, timezone

from bs4 import BeautifulSoup
from confluence_client import ConfluenceClient, cell, link_cell, get_or_create_table
from jira_client import build_jql, fetch_issues

STATUS_HEADERS = ["Issue", "Summary", "Status", "Assignee", "Fix Version", "Age (days)", "SLA", "Blocked Reason", "Dependency Ticket(s)", "Updated", "Link"]
ALERT_HEADERS = ["Severity", "Alert", "Details", "Detected At"]
OWNERSHIP_HEADERS = ["Assignee", "Assigned Total", "In Progress", "Blocked", "SLA Breaches"]
READINESS_HEADERS = ["Date (UTC)", "Total Issues", "Done", "In Progress", "Blocked", "Open Regressions", "Readiness Score", "Scope Drift"]
ANOMALY_HEADERS = ["Issue", "Type", "Reason", "Detected At"]
TREND_HEADERS = ["Date (UTC)", "Total", "Done", "In Progress", "Blocked", "Overdue", "Added", "Removed"]
AUDIT_HEADERS = ["Date (UTC)", "Job", "Fetched", "Inserted", "Updated", "Removed From Scope", "SLA Breaches", "Result"]

ENV_ALIASES = {
    "CONFLUENCE_BASE_URL": ("TRACKER_CONFLUENCE_BASE_URL", "CONFLUENCE_BASE_URL"),
    "CONFLUENCE_EMAIL": ("TRACKER_CONFLUENCE_EMAIL", "CONFLUENCE_EMAIL"),
    "CONFLUENCE_API_TOKEN": ("TRACKER_CONFLUENCE_API_TOKEN", "CONFLUENCE_API_TOKEN"),
    "CONFLUENCE_JIRA_PAGE_ID": ("TRACKER_CONFLUENCE_JIRA_PAGE_ID", "CONFLUENCE_JIRA_PAGE_ID"),
    "JIRA_BASE_URL": ("TRACKER_JIRA_BASE_URL", "JIRA_BASE_URL"),
    "JIRA_EMAIL": ("TRACKER_JIRA_EMAIL", "JIRA_EMAIL"),
    "JIRA_API_TOKEN": ("TRACKER_JIRA_API_TOKEN", "JIRA_API_TOKEN"),
    "JIRA_JQL": ("TRACKER_JIRA_JQL", "JIRA_JQL"),
    "JIRA_TEAM_NAME": ("TRACKER_JIRA_TEAM_NAME", "JIRA_TEAM_NAME"),
    "JIRA_TEAM_FIELD": ("TRACKER_JIRA_TEAM_FIELD", "JIRA_TEAM_FIELD"),
    "SLA_IN_PROGRESS_DAYS": ("TRACKER_SLA_IN_PROGRESS_DAYS", "SLA_IN_PROGRESS_DAYS"),
    "SLA_BLOCKED_DAYS": ("TRACKER_SLA_BLOCKED_DAYS", "SLA_BLOCKED_DAYS"),
    "BLOCKED_STATUSES": ("TRACKER_BLOCKED_STATUSES", "BLOCKED_STATUSES"),
    "DONE_STATUSES": ("TRACKER_DONE_STATUSES", "DONE_STATUSES"),
    "BLOCKED_REASON_FIELD": ("TRACKER_BLOCKED_REASON_FIELD", "BLOCKED_REASON_FIELD"),
    "DEPENDENCY_LINK_TYPES": ("TRACKER_DEPENDENCY_LINK_TYPES", "DEPENDENCY_LINK_TYPES"),
    "REMOVE_STALE_ROWS": ("TRACKER_REMOVE_STALE_ROWS", "REMOVE_STALE_ROWS"),
    "KEEP_SNAPSHOT_ROWS": ("TRACKER_KEEP_SNAPSHOT_ROWS", "KEEP_SNAPSHOT_ROWS"),
    "OPEN_REGRESSION_COUNT": ("TRACKER_OPEN_REGRESSION_COUNT", "OPEN_REGRESSION_COUNT"),
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


def parse_csv_set(value, default_csv):
    raw = value.strip() if value else default_csv
    return {x.strip().lower() for x in raw.split(",") if x.strip()}


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


def now_utc_text():
    return datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")


def fetch_jira_issues(base_url, email, token, jql):
    fields = ["summary", "status", "assignee", "fixVersions", "updated", "statuscategorychangedate", "issuelinks"]
    blocked_reason_field = cfg("BLOCKED_REASON_FIELD")
    if blocked_reason_field:
        fields.append(blocked_reason_field)
    return fetch_issues(base_url, email, token, jql, fields)


def extract_dependency_keys(issue, allowed_link_types):
    deps = []
    for link in issue["fields"].get("issuelinks", []) or []:
        link_type_name = (link.get("type", {}).get("name", "") or "").strip().lower()
        if allowed_link_types and link_type_name not in allowed_link_types:
            continue
        linked = link.get("outwardIssue") or link.get("inwardIssue")
        if linked and linked.get("key"):
            deps.append(linked["key"])
    seen = set()
    ordered = []
    for k in deps:
        if k not in seen:
            seen.add(k)
            ordered.append(k)
    return ordered


def issue_to_row(issue, jira_base_url, settings):
    fields = issue["fields"]
    assignee = fields.get("assignee")
    fix_versions = fields.get("fixVersions") or []
    status_name = fields.get("status", {}).get("name", "") or ""
    status_norm = status_name.strip().lower()

    baseline_dt = parse_jira_datetime(fields.get("statuscategorychangedate")) or parse_jira_datetime(fields.get("updated"))
    age_days = max(0, (datetime.now(timezone.utc) - baseline_dt.astimezone(timezone.utc)).days) if baseline_dt else 0

    blocked_reason = "-"
    if settings["blocked_reason_field"]:
        value = fields.get(settings["blocked_reason_field"])
        if isinstance(value, dict):
            blocked_reason = value.get("value") or value.get("name") or str(value)
        elif isinstance(value, list):
            blocked_reason = ", ".join(str(v.get("value") if isinstance(v, dict) else v) for v in value if v) or "-"
        elif value:
            blocked_reason = str(value)

    dependencies = extract_dependency_keys(issue, settings["dependency_link_types"])
    is_done = status_norm in settings["done_statuses"]
    is_blocked = status_norm in settings["blocked_statuses"]
    in_progress = status_norm not in settings["done_statuses"] and not is_blocked

    sla = "OK"
    sla_breach = False
    if not is_done:
        if is_blocked and age_days > settings["sla_blocked_days"]:
            sla = f"BREACH (> {settings['sla_blocked_days']}d blocked)"
            sla_breach = True
        elif in_progress and age_days > settings["sla_in_progress_days"]:
            sla = f"BREACH (> {settings['sla_in_progress_days']}d in progress)"
            sla_breach = True

    return {
        "key": issue["key"],
        "summary": (fields.get("summary") or "")[:180],
        "status": status_name,
        "assignee": assignee["displayName"] if assignee else "Unassigned",
        "fix_version": ", ".join(v.get("name", "") for v in fix_versions if v.get("name")) or "—",
        "age_days": age_days,
        "sla": sla,
        "sla_breach": sla_breach,
        "blocked_reason": blocked_reason if blocked_reason else "-",
        "dependencies": ", ".join(dependencies) if dependencies else "-",
        "updated": (fields.get("updated") or "")[:16].replace("T", " "),
        "link": f"{jira_base_url.rstrip('/')}/browse/{issue['key']}",
        "is_done": is_done,
        "is_blocked": is_blocked,
        "in_progress": in_progress,
    }


def make_row_html(row):
    return (
        "<tr>"
        + cell(row["key"]) + cell(row["summary"]) + cell(row["status"])
        + cell(row["assignee"]) + cell(row["fix_version"]) + cell(row["age_days"])
        + cell(row["sla"]) + cell(row["blocked_reason"]) + cell(row["dependencies"])
        + cell(row["updated"]) + link_cell(row["link"], "Jira")
        + "</tr>"
    )


def make_generic_row_html(values):
    return "<tr>" + "".join(cell(v) for v in values) + "</tr>"


def upsert_rows(soup, rows_by_key, remove_stale=False):
    table = get_or_create_table(soup, STATUS_HEADERS, table_index=0)
    tbody = table.find("tbody") or table
    existing_rows = tbody.find_all("tr")
    header_row = existing_rows[0] if existing_rows else None
    data_rows = existing_rows[1:] if header_row else existing_rows

    existing_keys = []
    existing_by_key = {}
    for tr in data_rows:
        first_cell = tr.find(["td", "th"])
        key = first_cell.get_text(strip=True) if first_cell else ""
        if key:
            existing_keys.append(key)
            existing_by_key[key] = tr

    inserted = 0
    updated = 0
    for key, row in rows_by_key.items():
        new_tr = BeautifulSoup(make_row_html(row), "html.parser").find("tr")
        if key in existing_by_key:
            old_tr = existing_by_key[key]
            if str(old_tr) != str(new_tr):
                old_tr.replace_with(new_tr)
                updated += 1
        else:
            if header_row:
                header_row.insert_after(new_tr)
                header_row = new_tr
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

    return {"inserted": inserted, "updated": updated, "removed_scope": removed_scope}


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
    rows = tbody.find_all("tr")
    if not rows:
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
    page_id = cfg("CONFLUENCE_JIRA_PAGE_ID")
    jira_base_url, jira_email, jira_token = cfg("JIRA_BASE_URL"), cfg("JIRA_EMAIL"), cfg("JIRA_API_TOKEN")
    base_jql = cfg("JIRA_JQL")
    team_name = cfg("JIRA_TEAM_NAME")
    team_field = cfg("JIRA_TEAM_FIELD", '"Team[Team]"')
    jql = build_jql(base_jql, team_name, team_field)

    settings = {
        "sla_in_progress_days": as_int(cfg("SLA_IN_PROGRESS_DAYS", "7"), 7),
        "sla_blocked_days": as_int(cfg("SLA_BLOCKED_DAYS", "3"), 3),
        "blocked_statuses": parse_csv_set(cfg("BLOCKED_STATUSES"), "Blocked,On Hold"),
        "done_statuses": parse_csv_set(cfg("DONE_STATUSES"), "Done,Closed,Resolved"),
        "blocked_reason_field": cfg("BLOCKED_REASON_FIELD"),
        "dependency_link_types": parse_csv_set(cfg("DEPENDENCY_LINK_TYPES"), "Blocks,Depends,Relates"),
        "remove_stale_rows": cfg("REMOVE_STALE_ROWS", "false").strip().lower() in {"1", "true", "yes", "on"},
        "keep_snapshot_rows": as_int(cfg("KEEP_SNAPSHOT_ROWS", "120"), 120),
        "open_regression_count": as_int(cfg("OPEN_REGRESSION_COUNT", "0"), 0),
    }

    missing = [n for n, v in [
        ("CONFLUENCE_BASE_URL", base_url), ("CONFLUENCE_EMAIL", email),
        ("CONFLUENCE_API_TOKEN", token), ("CONFLUENCE_JIRA_PAGE_ID", page_id),
        ("JIRA_BASE_URL", jira_base_url), ("JIRA_EMAIL", jira_email),
        ("JIRA_API_TOKEN", jira_token), ("JIRA_JQL", jql),
    ] if not v]
    if missing:
        print(f"ERROR: missing env vars: {', '.join(missing)}", file=sys.stderr)
        sys.exit(1)

    issues = fetch_jira_issues(jira_base_url, jira_email, jira_token, jql)
    print(f"Fetched {len(issues)} issues for JQL: {jql}")
    rows_by_key = {i["key"]: issue_to_row(i, jira_base_url, settings) for i in issues}

    client = ConfluenceClient(base_url, email, token)
    page = client.get_page(page_id)
    title = page["title"]
    version = page["version"]["number"]
    body = page.get("body", {}).get("storage", {}).get("value", "")
    soup = BeautifulSoup(body or "", "html.parser")

    stats = upsert_rows(soup, rows_by_key, remove_stale=settings["remove_stale_rows"])
    overdue = sum(1 for r in rows_by_key.values() if r["sla_breach"])
    done = sum(1 for r in rows_by_key.values() if r["is_done"])
    in_progress = sum(1 for r in rows_by_key.values() if r["in_progress"])
    blocked = sum(1 for r in rows_by_key.values() if r["is_blocked"])
    removed_count = len(stats["removed_scope"])

    now = now_utc_text()
    alerts = []
    anomalies = []
    ownership = defaultdict(lambda: {"total": 0, "in_progress": 0, "blocked": 0, "breaches": 0})
    for row in rows_by_key.values():
        owner = row["assignee"]
        ownership[owner]["total"] += 1
        if row["in_progress"]:
            ownership[owner]["in_progress"] += 1
        if row["is_blocked"]:
            ownership[owner]["blocked"] += 1
            alerts.append(["medium", "Blocked issue", f"{row['key']} blocked | deps: {row['dependencies']}", now])
        if row["sla_breach"]:
            ownership[owner]["breaches"] += 1
            alerts.append(["high", "SLA breach", f"{row['key']} - {row['sla']}", now])
        if row["assignee"] == "Unassigned":
            anomalies.append([row["key"], "Data Quality", "Assignee is missing", now])
        if row["fix_version"] == "—":
            anomalies.append([row["key"], "Data Quality", "Fix Version is missing", now])
        if row["is_blocked"] and row["blocked_reason"] in {"", "-"}:
            anomalies.append([row["key"], "Blocked Context", "Blocked status without blocked reason", now])

    ownership_rows = [[o, m["total"], m["in_progress"], m["blocked"], m["breaches"]] for o, m in sorted(ownership.items())]
    replace_table_rows(soup, 1, ALERT_HEADERS, alerts)
    replace_table_rows(soup, 2, OWNERSHIP_HEADERS, ownership_rows)

    total = len(rows_by_key)
    done_ratio = (done / total * 100.0) if total else 0.0
    readiness_score = max(0.0, min(100.0, round(done_ratio - (blocked * 5) - (settings["open_regression_count"] * 10), 1)))
    append_snapshot_row(soup, 3, READINESS_HEADERS, [now, total, done, in_progress, blocked, settings["open_regression_count"], readiness_score, f"+{stats['inserted']}/-{removed_count}"], settings["keep_snapshot_rows"])

    replace_table_rows(soup, 4, ANOMALY_HEADERS, anomalies)
    append_snapshot_row(soup, 5, TREND_HEADERS, [now, total, done, in_progress, blocked, overdue, stats["inserted"], removed_count], settings["keep_snapshot_rows"])
    append_snapshot_row(soup, 6, AUDIT_HEADERS, [now, "jira-status-to-confluence", len(rows_by_key), stats["inserted"], stats["updated"], removed_count, overdue, "success"], settings["keep_snapshot_rows"])

    updated_body = str(soup)
    if updated_body == body:
        print("No changes to Confluence page content — skipping update.")
        return

    client.update_page_with_retry(page_id, title, updated_body, version + 1, message="Jira delivery status sync")
    print(f"Confluence page '{title}' updated with advanced delivery sync.")


if __name__ == "__main__":
    main()
