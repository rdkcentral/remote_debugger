"""Shared Jira Cloud REST API v3 helpers used by all sync scripts."""

import requests


def build_jql(base_jql, team_name, team_field):
    """AND an optional team filter onto a base JQL string."""
    if not team_name:
        return base_jql
    escaped_team = team_name.replace('"', '\\"')
    return f'({base_jql}) AND {team_field} = "{escaped_team}"'


def fetch_issues(base_url, email, token, jql, fields):
    """Paginated JQL search against /rest/api/3/search/jql."""
    url = f"{base_url.rstrip('/')}/rest/api/3/search/jql"
    all_issues = []
    next_token = None
    while True:
        payload = {"jql": jql, "fields": fields, "maxResults": 100}
        if next_token:
            payload["nextPageToken"] = next_token
        resp = requests.post(url, json=payload, auth=(email, token),
                              headers={"Content-Type": "application/json"})
        resp.raise_for_status()
        data = resp.json()
        all_issues.extend(data.get("issues", []))
        next_token = data.get("nextPageToken")
        if not next_token:
            break
    return all_issues
