#!/usr/bin/env python3
"""
Regression Impact Checker
=========================
Given a culprit commit SHA, determines which tracked remote branches
(main, develop, release/*, support/*) contain that commit and prints a
categorised impact report.

Optionally posts a note row to the Confluence tracker page so the impact
finding is recorded alongside the normal commit/release audit trail.

Usage
-----
Local (requires a git repo with remote refs fetched):
    python scripts/check_regression_impact.py --sha <sha> [--ticket RDKB-1234]
    python scripts/check_regression_impact.py --sha <sha> --post-confluence

GitHub Actions (workflow_dispatch):
    Set inputs sha, ticket in .github/workflows/regression-impact.yml
    and the workflow sets REGRESSION_SHA / REGRESSION_TICKET env vars.

Required env vars for --post-confluence (same as the main tracker):
    TRACKER_CONFLUENCE_BASE_URL
    TRACKER_CONFLUENCE_EMAIL
    TRACKER_CONFLUENCE_API_TOKEN
    TRACKER_CONFLUENCE_PAGE_ID

Optional env vars:
    REGRESSION_SHA      fallback when --sha is not passed (set by workflow)
    REGRESSION_TICKET   fallback when --ticket is not passed (set by workflow)
    TRACKER_GITHUB_REPOSITORY   used for output links (owner/repo)
    TRACKER_GITHUB_SERVER_URL   defaults to https://github.com
"""

import argparse
import html
import os
import subprocess
import sys
from datetime import datetime, timezone

# ---------------------------------------------------------------------------
# Branch category helpers
# ---------------------------------------------------------------------------
TRACKED_PREFIXES = {
    "main": "main branches",
    "develop": "develop branches",
    "release": "release branches",
    "support": "support branches",
}


def _category(branch_name: str) -> str:
    """Return a human-readable category label for a branch name."""
    for prefix, label in TRACKED_PREFIXES.items():
        if branch_name == prefix or branch_name.startswith(f"{prefix}/"):
            return label
    return "other branches"


def _is_tracked(branch_name: str) -> bool:
    for prefix in TRACKED_PREFIXES:
        if branch_name == prefix or branch_name.startswith(f"{prefix}/"):
            return True
    return False


# ---------------------------------------------------------------------------
# Git helpers
# ---------------------------------------------------------------------------
def resolve_full_sha(sha: str) -> str:
    """Expand a short SHA to the full 40-char form."""
    try:
        result = subprocess.run(
            ["git", "rev-parse", sha],
            capture_output=True, text=True, check=True,
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return sha


def get_remote_branches_containing(sha: str) -> list[str]:
    """
    Return remote branch names (without origin/ prefix) that contain sha.
    Requires remote refs to be locally fetched (git fetch --all).
    """
    try:
        result = subprocess.run(
            ["git", "branch", "-r", "--contains", sha],
            capture_output=True, text=True, check=True,
        )
    except subprocess.CalledProcessError as exc:
        print(f"ERROR: git branch failed: {exc.stderr.strip()}", file=sys.stderr)
        return []

    branches = []
    for line in result.stdout.splitlines():
        name = line.strip()
        # Strip remote prefix (origin/, upstream/, etc.)
        if "/" in name:
            name = name.split("/", 1)[1]
        if name and "->" not in name:
            branches.append(name)
    return sorted(set(branches))


def get_all_tracked_remote_branches() -> list[str]:
    """Return all remote branches matching tracked prefixes."""
    try:
        result = subprocess.run(
            ["git", "branch", "-r"],
            capture_output=True, text=True, check=True,
        )
    except subprocess.CalledProcessError:
        return []

    branches = []
    for line in result.stdout.splitlines():
        name = line.strip()
        if "/" in name:
            name = name.split("/", 1)[1]
        if name and "->" not in name and _is_tracked(name):
            branches.append(name)
    return sorted(set(branches))


# ---------------------------------------------------------------------------
# Report builder
# ---------------------------------------------------------------------------
def build_report(sha: str, full_sha: str, ticket: str) -> dict:
    """
    Returns a dict:
        impacted   : {category_label: [branch, ...]}
        not_impacted: [branch, ...]
        has_hotfix_branches: bool
    """
    containing = set(get_remote_branches_containing(sha))
    all_tracked = set(get_all_tracked_remote_branches())
    not_impacted = sorted(b for b in all_tracked if b not in containing)

    impacted: dict[str, list[str]] = {}
    for branch in sorted(containing):
        cat = _category(branch)
        impacted.setdefault(cat, []).append(branch)

    hotfix_needed = [
        b for b in containing
        if b.startswith("release/") or b.startswith("support/")
    ]

    return {
        "sha": sha,
        "full_sha": full_sha,
        "ticket": ticket,
        "impacted": impacted,
        "not_impacted": not_impacted,
        "hotfix_branches": hotfix_needed,
    }


def print_report(report: dict) -> None:
    sha = report["sha"]
    full_sha = report["full_sha"]
    ticket = report["ticket"]
    impacted = report["impacted"]
    not_impacted = report["not_impacted"]
    hotfix_branches = report["hotfix_branches"]

    width = 68
    print("=" * width)
    print(f"Regression Impact Report — SHA: {sha[:7]}  (full: {full_sha[:12]}...)")
    if ticket:
        print(f"Ticket: {ticket}")
    print("=" * width)

    if impacted:
        print("\nIMPACTED BRANCHES (contain this commit)")
        for label in ["main branches", "develop branches", "release branches", "support branches", "other branches"]:
            branches = impacted.get(label)
            if branches:
                print(f"  {label:<20}: {', '.join(branches)}")
    else:
        print("\nNo tracked branches contain this commit.")

    if not_impacted:
        print(f"\nNOT IMPACTED       : {', '.join(not_impacted)}")

    print()
    if hotfix_branches:
        print(f"⚠  Action needed: hotfix required on {', '.join(hotfix_branches)}")
    elif impacted:
        print("✓  No release/support branches impacted — hotfix may not be required.")
    print("=" * width)


# ---------------------------------------------------------------------------
# Confluence integration
# ---------------------------------------------------------------------------
def _cfg(key: str) -> str:
    return os.environ.get(f"TRACKER_{key}", os.environ.get(key, ""))


def post_to_confluence(report: dict) -> None:
    try:
        import requests
        from bs4 import BeautifulSoup
    except ImportError:
        print("ERROR: requests/beautifulsoup4 not installed. Run: pip install -r scripts/requirements.txt",
              file=sys.stderr)
        sys.exit(1)

    base_url = _cfg("CONFLUENCE_BASE_URL").rstrip("/")
    email    = _cfg("CONFLUENCE_EMAIL")
    token    = _cfg("CONFLUENCE_API_TOKEN")
    page_id  = _cfg("CONFLUENCE_PAGE_ID")
    repo     = _cfg("GITHUB_REPOSITORY")
    server   = _cfg("GITHUB_SERVER_URL") or "https://github.com"

    missing = [n for n, v in [
        ("TRACKER_CONFLUENCE_BASE_URL", base_url),
        ("TRACKER_CONFLUENCE_EMAIL", email),
        ("TRACKER_CONFLUENCE_API_TOKEN", token),
        ("TRACKER_CONFLUENCE_PAGE_ID", page_id),
    ] if not v]
    if missing:
        print(f"ERROR: missing env vars for Confluence post: {', '.join(missing)}", file=sys.stderr)
        sys.exit(1)

    sha       = report["sha"]
    full_sha  = report["full_sha"]
    ticket    = report["ticket"]
    impacted  = report["impacted"]
    hotfix    = report["hotfix_branches"]
    now       = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M")

    # Build human-readable impact summary
    impact_lines = []
    for label, branches in impacted.items():
        impact_lines.append(f"{label}: {', '.join(branches)}")
    impact_text  = " | ".join(impact_lines) if impact_lines else "No tracked branches impacted"
    action_text  = (
        f"Hotfix required on: {', '.join(hotfix)}"
        if hotfix else
        "No release/support branches impacted"
    )
    commit_link  = f"{server}/{repo}/commit/{full_sha}" if repo else "#"
    ticket_html  = f'<a href="#">{html.escape(ticket)}</a>' if ticket else "-"
    sha_html     = f'<a href="{html.escape(commit_link)}">{html.escape(sha[:7])}</a>'

    # Build a note row for the existing tracker table structure
    # Uses a colspan=9 single cell so it fits any table width
    note_row_html = (
        "<tr>"
        f'<td><p>{html.escape(now)}</p></td>'
        f'<td><p>🔍 Regression</p></td>'
        f'<td><p>{sha_html}</p></td>'
        f'<td><p>{ticket_html}</p></td>'
        f'<td colspan="5">'
        f'<p><strong>Impact:</strong> {html.escape(impact_text)}</p>'
        f'<p><strong>Action:</strong> {html.escape(action_text)}</p>'
        "</td>"
        "</tr>"
    )

    auth    = (email, token)
    headers = {"Content-Type": "application/json"}
    session = requests.Session()
    session.auth  = auth
    session.headers.update(headers)

    # Fetch the page
    resp = session.get(
        f"{base_url}/api/v2/pages/{page_id}",
        params={"body-format": "storage"},
    )
    resp.raise_for_status()
    page          = resp.json()
    title         = page["title"]
    version       = page["version"]["number"]
    current_body  = page.get("body", {}).get("storage", {}).get("value", "")

    # Insert note row at the top of the first tracker table on the page
    soup = BeautifulSoup(current_body, "html.parser")
    table = soup.find("table")
    if table:
        tbody = table.find("tbody") or table
        header_row = tbody.find("tr")
        new_row = BeautifulSoup(note_row_html, "html.parser").find("tr")
        if header_row:
            header_row.insert_after(new_row)
        else:
            tbody.insert(0, new_row)
        updated_body = str(soup)
    else:
        print("WARNING: no table found on Confluence page — skipping row insert.", file=sys.stderr)
        updated_body = current_body

    payload = {
        "id": str(page_id),
        "status": "current",
        "title": title,
        "body": {"representation": "storage", "value": updated_body},
        "version": {"number": version + 1, "message": f"Regression impact note: {sha[:7]}"},
    }
    put_resp = session.put(f"{base_url}/api/v2/pages/{page_id}", json=payload)
    put_resp.raise_for_status()
    print(f"✓ Confluence page '{title}' updated to version {version + 1} with regression note.")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
def main() -> None:
    parser = argparse.ArgumentParser(
        description="Check which tracked branches are impacted by a regression commit."
    )
    parser.add_argument(
        "--sha",
        default=os.environ.get("REGRESSION_SHA", ""),
        help="Culprit commit SHA (short or full). Also reads REGRESSION_SHA env var.",
    )
    parser.add_argument(
        "--ticket",
        default=os.environ.get("REGRESSION_TICKET", ""),
        help="Optional Jira ticket ID for the regression (e.g. RDKB-1234).",
    )
    parser.add_argument(
        "--post-confluence",
        action="store_true",
        help="Post an impact note row to the Confluence tracker page.",
    )
    args = parser.parse_args()

    if not args.sha:
        parser.error("--sha is required (or set REGRESSION_SHA env var).")

    print(f"Fetching remote refs... (git fetch --all)")
    subprocess.run(["git", "fetch", "--all", "--quiet"], check=False)

    full_sha = resolve_full_sha(args.sha)
    report   = build_report(args.sha, full_sha, args.ticket)

    print_report(report)

    if args.post_confluence:
        post_to_confluence(report)


if __name__ == "__main__":
    main()
