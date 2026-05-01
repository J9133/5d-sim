"""
Anki Statistics Analyzer - via AnkiConnect
============================================
Requirements:
  - Anki open + AnkiConnect addon (code: 2055492159)
  - pip install requests tabulate colorama
  - AnkiConnect default port: 8765
"""

import requests
import json
from datetime import datetime, date, timedelta
from collections import defaultdict
import sys

try:
    from tabulate import tabulate
    from colorama import init, Fore, Style
    init(autoreset=True)
except ImportError:
    print("Installing dependencies...")
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "requests", "tabulate", "colorama", "-q"])
    from tabulate import tabulate
    from colorama import init, Fore, Style
    init(autoreset=True)


# ─────────────────────────────────────────────
#  AnkiConnect helper
# ─────────────────────────────────────────────
ANKI_URL = "http://localhost:8765"

def anki(action: str, **params):
    payload = {"action": action, "version": 6, "params": params}
    try:
        r = requests.post(ANKI_URL, json=payload, timeout=5)
        r.raise_for_status()
        result = r.json()
        if result.get("error"):
            raise RuntimeError(f"AnkiConnect error: {result['error']}")
        return result["result"]
    except requests.exceptions.ConnectionError:
        print(f"\n{Fore.RED}✗ Cannot connect to Anki.")
        print(f"  Make sure Anki is open and AnkiConnect addon is installed.")
        print(f"  Addon code: 2055492159  →  Tools > Add-ons > Get Add-ons{Style.RESET_ALL}\n")
        sys.exit(1)


# ─────────────────────────────────────────────
#  Deck selector
# ─────────────────────────────────────────────
def choose_deck() -> str:
    decks = anki("deckNames")
    decks = [d for d in decks if d != "Default"]
    decks.sort()

    print(f"\n{Fore.CYAN}{'═'*55}")
    print(f"  📚  Anki Statistics Analyzer")
    print(f"{'═'*55}{Style.RESET_ALL}\n")
    print(f"{Fore.YELLOW}Available decks:{Style.RESET_ALL}\n")

    for i, d in enumerate(decks, 1):
        print(f"  {Fore.WHITE}{i:>2}.{Style.RESET_ALL} {d}")

    print()
    while True:
        raw = input(f"{Fore.CYAN}Enter deck number or name: {Style.RESET_ALL}").strip()
        if raw.isdigit():
            idx = int(raw) - 1
            if 0 <= idx < len(decks):
                return decks[idx]
        else:
            matches = [d for d in decks if raw.lower() in d.lower()]
            if len(matches) == 1:
                return matches[0]
            elif len(matches) > 1:
                print(f"{Fore.YELLOW}  Multiple matches: {', '.join(matches)}{Style.RESET_ALL}")
                continue
        print(f"{Fore.RED}  Invalid input, try again.{Style.RESET_ALL}")


# ─────────────────────────────────────────────
#  Fetch review log for a deck
# ─────────────────────────────────────────────
def get_reviews(deck: str) -> list[dict]:
    """
    Returns list of review records for cards in the given deck.
    Each record: { date, reviews, time_ms }
    Grouped by calendar day.
    """
    # Get all card IDs in deck (including subdecks)
    card_ids = anki("findCards", query=f'deck:"{deck}"')
    if not card_ids:
        return []

    # Pull review log for each card (getReviewsOfCards)
    # AnkiConnect returns: { card_id: [ [id, usn, ease, ivl, lastIvl, factor, time, type], ... ] }
    reviews_map = anki("getReviewsOfCards", cards=card_ids)

    # Aggregate by date
    day_data: dict[str, dict] = defaultdict(lambda: {"reviews": 0, "time_ms": 0})

    for card_id_str, entries in reviews_map.items():
        for entry in entries:
            # AnkiConnect returns each entry as a dict:
            # { "id": ms_timestamp, "usn": .., "ease": .., "ivl": ..,
            #   "lastIvl": .., "factor": .., "time": ms_taken, "type": .. }
            if isinstance(entry, dict):
                ts_ms   = entry.get("id", 0)
                time_ms = entry.get("time", 0)
            else:
                # fallback: list format [id, usn, ease, ivl, lastIvl, factor, time, type]
                ts_ms   = entry[0]
                time_ms = entry[6]
            if ts_ms <= 0:
                continue
            review_date = datetime.fromtimestamp(ts_ms / 1000).strftime("%Y-%m-%d")
            day_data[review_date]["reviews"] += 1
            day_data[review_date]["time_ms"] += time_ms

    # Build sorted list
    rows = []
    for day, vals in sorted(day_data.items()):
        rows.append({
            "date":     day,
            "reviews":  vals["reviews"],
            "time_ms":  vals["time_ms"],
            "time_min": vals["time_ms"] / 60000,
        })
    return rows


# ─────────────────────────────────────────────
#  Format helpers
# ─────────────────────────────────────────────
def fmt_time(ms: float) -> str:
    """Format milliseconds → human-readable."""
    secs = int(ms / 1000)
    if secs < 60:
        return f"{secs}s"
    mins = secs // 60
    secs %= 60
    if mins < 60:
        return f"{mins}m {secs:02d}s"
    hrs = mins // 60
    mins %= 60
    return f"{hrs}h {mins:02d}m"

def efficiency_bar(ratio: float, max_ratio: float, width: int = 12) -> str:
    """Visual bar for reviews/min ratio."""
    if max_ratio == 0:
        return "─" * width
    filled = int((ratio / max_ratio) * width)
    bar = "█" * filled + "░" * (width - filled)
    return bar

def color_ratio(ratio: float, p25: float, p75: float) -> str:
    """Color-code the ratio: red=slow, yellow=mid, green=fast."""
    if ratio >= p75:
        return Fore.GREEN
    elif ratio >= p25:
        return Fore.YELLOW
    return Fore.RED


# ─────────────────────────────────────────────
#  Print summary cards
# ─────────────────────────────────────────────
def print_summary(deck: str, rows: list[dict]):
    total_reviews = sum(r["reviews"] for r in rows)
    total_time_ms = sum(r["time_ms"] for r in rows)
    total_days    = len(rows)

    avg_reviews   = total_reviews / total_days if total_days else 0
    avg_time_min  = (total_time_ms / 60000) / total_days if total_days else 0
    overall_ratio = total_reviews / (total_time_ms / 60000) if total_time_ms else 0

    print(f"\n{Fore.CYAN}{'─'*55}")
    print(f"  Deck: {Fore.WHITE}{deck}{Style.RESET_ALL}")
    print(f"{Fore.CYAN}{'─'*55}{Style.RESET_ALL}")

    cards = [
        ("📅 Days studied",    f"{total_days}"),
        ("🃏 Total reviews",   f"{total_reviews:,}"),
        ("⏱  Total time",      fmt_time(total_time_ms)),
        ("📊 Avg reviews/day", f"{avg_reviews:.1f}"),
        ("⏳ Avg time/day",    fmt_time(avg_time_min * 60000)),
        ("⚡ Overall speed",   f"{overall_ratio:.2f} reviews/min"),
    ]

    for label, val in cards:
        print(f"  {Fore.YELLOW}{label:<22}{Style.RESET_ALL}  {Fore.WHITE}{val}{Style.RESET_ALL}")

    print(f"{Fore.CYAN}{'─'*55}{Style.RESET_ALL}\n")


# ─────────────────────────────────────────────
#  Print daily table
# ─────────────────────────────────────────────
def print_daily_table(rows: list[dict]):
    if not rows:
        print(f"{Fore.RED}No reviews found for this deck.{Style.RESET_ALL}")
        return

    ratios = [r["reviews"] / r["time_min"] for r in rows if r["time_min"] > 0]
    if not ratios:
        return

    ratios_sorted = sorted(ratios)
    p25 = ratios_sorted[len(ratios_sorted) // 4]
    p75 = ratios_sorted[(len(ratios_sorted) * 3) // 4]
    max_ratio = max(ratios)

    print(f"\n{Fore.CYAN}  📋  Daily Breakdown{Style.RESET_ALL}")
    print(f"{Fore.CYAN}{'─'*75}{Style.RESET_ALL}")

    header = f"  {'DATE':<12} {'REVIEWS':>8}  {'TIME':>9}  {'SPEED (rev/min)':>16}  {'BAR':<14}"
    print(f"{Fore.WHITE}{header}{Style.RESET_ALL}")
    print(f"{Fore.CYAN}{'─'*75}{Style.RESET_ALL}")

    for r in rows:
        if r["time_min"] <= 0:
            continue

        ratio = r["reviews"] / r["time_min"]
        clr   = color_ratio(ratio, p25, p75)
        bar   = efficiency_bar(ratio, max_ratio)

        # Highlight today / yesterday
        day_label = r["date"]
        if r["date"] == date.today().strftime("%Y-%m-%d"):
            day_label = f"{Fore.CYAN}{r['date']} ◀{Style.RESET_ALL}"
        elif r["date"] == (date.today() - timedelta(1)).strftime("%Y-%m-%d"):
            day_label = f"{Fore.MAGENTA}{r['date']}{Style.RESET_ALL}"

        print(
            f"  {day_label:<12} "
            f"{Fore.WHITE}{r['reviews']:>8,}{Style.RESET_ALL}  "
            f"{Fore.WHITE}{fmt_time(r['time_ms']):>9}{Style.RESET_ALL}  "
            f"{clr}{ratio:>14.2f}  {Style.RESET_ALL}"
            f"{clr}{bar}{Style.RESET_ALL}"
        )

    print(f"{Fore.CYAN}{'─'*75}{Style.RESET_ALL}")

    # Legend
    print(f"\n  Legend:  "
          f"{Fore.GREEN}█ Fast (top 25%){Style.RESET_ALL}  "
          f"{Fore.YELLOW}█ Average{Style.RESET_ALL}  "
          f"{Fore.RED}█ Slow (bottom 25%){Style.RESET_ALL}  "
          f"  ◀ Today")


# ─────────────────────────────────────────────
#  Filter options
# ─────────────────────────────────────────────
def ask_filter(rows: list[dict]) -> list[dict]:
    print(f"\n{Fore.YELLOW}Filter by date range?{Style.RESET_ALL}")
    print("  1. All time")
    print("  2. Last 7 days")
    print("  3. Last 30 days")
    print("  4. This month")
    print("  5. Custom range")

    choice = input(f"\n{Fore.CYAN}Choose [1-5, default=1]: {Style.RESET_ALL}").strip() or "1"

    today = date.today()
    if choice == "2":
        cutoff = today - timedelta(days=7)
    elif choice == "3":
        cutoff = today - timedelta(days=30)
    elif choice == "4":
        cutoff = today.replace(day=1)
    elif choice == "5":
        start = input("  Start date (YYYY-MM-DD): ").strip()
        end   = input("  End date   (YYYY-MM-DD): ").strip()
        return [r for r in rows if start <= r["date"] <= end]
    else:
        return rows

    cutoff_str = cutoff.strftime("%Y-%m-%d")
    return [r for r in rows if r["date"] >= cutoff_str]


# ─────────────────────────────────────────────
#  Main
# ─────────────────────────────────────────────
def main():
    deck  = choose_deck()
    print(f"\n{Fore.CYAN}⏳ Fetching review data for: {Fore.WHITE}{deck}{Style.RESET_ALL}")

    rows = get_reviews(deck)
    if not rows:
        print(f"{Fore.RED}No review history found for this deck.{Style.RESET_ALL}")
        return

    filtered = ask_filter(rows)
    if not filtered:
        print(f"{Fore.RED}No data in the selected range.{Style.RESET_ALL}")
        return

    print_summary(deck, filtered)
    print_daily_table(filtered)

    # Export option
    print()
    export = input(f"{Fore.CYAN}Export to CSV? (y/N): {Style.RESET_ALL}").strip().lower()
    if export == "y":
        fname = f"anki_{deck.replace('/', '_').replace(' ','_')}.csv"
        with open(fname, "w", encoding="utf-8") as f:
            f.write("date,reviews,time_ms,time_min,reviews_per_min\n")
            for r in filtered:
                rpm = r["reviews"] / r["time_min"] if r["time_min"] > 0 else 0
                f.write(f"{r['date']},{r['reviews']},{r['time_ms']:.0f},{r['time_min']:.2f},{rpm:.3f}\n")
        print(f"{Fore.GREEN}  ✓ Saved: {fname}{Style.RESET_ALL}")


if __name__ == "__main__":
    main()