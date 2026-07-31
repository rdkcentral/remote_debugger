"""Shared Confluence Cloud REST API v2 client + storage-format table helpers."""

import html
import time

import requests
from bs4 import BeautifulSoup


class ConfluenceClient:
    def __init__(self, base_url, email, token):
        self.base_url = base_url.rstrip("/")
        self.session = requests.Session()
        self.session.auth = (email, token)
        self.session.headers.update({"Content-Type": "application/json"})

    def get_page(self, page_id):
        url = f"{self.base_url}/api/v2/pages/{page_id}"
        resp = self.session.get(url, params={"body-format": "storage"})
        resp.raise_for_status()
        return resp.json()

    def update_page(self, page_id, title, new_storage_value, new_version, message="Automated tracker update"):
        url = f"{self.base_url}/api/v2/pages/{page_id}"
        payload = {
            "id": str(page_id),
            "status": "current",
            "title": title,
            "body": {"representation": "storage", "value": new_storage_value},
            "version": {"number": new_version, "message": message},
        }
        resp = self.session.put(url, json=payload)
        resp.raise_for_status()
        return resp.json()

    def update_page_with_retry(
        self,
        page_id,
        title,
        new_storage_value,
        new_version,
        message="Automated tracker update",
        max_retries=4,
        initial_delay_seconds=1.0,
    ):
        """Update page with retry/backoff on version conflicts (HTTP 409)."""
        attempt = 0
        delay = initial_delay_seconds
        version = new_version

        while True:
            try:
                return self.update_page(page_id, title, new_storage_value, version, message=message)
            except requests.HTTPError as exc:
                status = getattr(exc.response, "status_code", None)
                if status != 409 or attempt >= max_retries:
                    raise

                attempt += 1
                print(
                    f"WARNING: Confluence version conflict (409). "
                    f"Retrying attempt {attempt}/{max_retries} after {delay:.1f}s."
                )
                time.sleep(delay)
                latest_page = self.get_page(page_id)
                version = latest_page["version"]["number"] + 1
                delay *= 2


def cell(text):
    return f"<td><p>{html.escape(str(text))}</p></td>"


def link_cell(url, label):
    return f'<td><p><a href="{html.escape(url)}">{html.escape(label)}</a></p></td>'


def header_cell(text):
    return f"<th><p><strong>{html.escape(text)}</strong></p></th>"


def build_empty_table(headers):
    header_row = "".join(header_cell(h) for h in headers)
    return f'<table data-layout="default"><tbody><tr>{header_row}</tr></tbody></table>'


def get_or_create_table(soup, headers, table_index=0):
    """Return the Nth table on the page, creating + inserting an empty one
    at the top of the page if fewer than table_index+1 tables exist."""
    tables = soup.find_all("table")
    while len(tables) <= table_index:
        new_table_soup = BeautifulSoup(build_empty_table(headers), "html.parser")
        new_table = new_table_soup.find("table")
        if soup.contents:
            soup.insert(0, new_table)
        else:
            soup.append(new_table)
        tables = soup.find_all("table")
    return tables[table_index]


def row_key(tr):
    """First cell's text content — used as the natural key for a row."""
    first_cell = tr.find(["td", "th"])
    return first_cell.get_text(strip=True) if first_cell else ""


def trim_rows(tbody, max_rows, has_header=True):
    rows = tbody.find_all("tr")
    data_rows = rows[1:] if has_header else rows
    if len(data_rows) > max_rows:
        for extra in data_rows[max_rows:]:
            extra.decompose()
