# COREVEX — Search. Browse. Think.

An AI-powered desktop web browser built in C++ with Qt 6. It has real multi-tab browsing, a custom search page that works without a search API key, an AI sidebar, and a library of advanced data structures (Hash Table, Trie, Binary Heap, AVL Tree, Inverted Index, Evidence Graph, BM25) that power search and ranking.

**Subject:** Advanced Data Structures (25CSH-202), Section 25BCS-801A, Group A  
**Submitted To:** MD Tauseef Sir  
**Submitted By:** Aarit Dhingra (25BCS10003), Dhruv (25BCS10405), Maithli (25BCS10376)

## How to Run (Sir, please read this first)

You only need **one double-click**. No code changes are needed.

**1. Install these once (Windows only)**
- Windows 10 or 11, 64-bit
- Visual Studio 2022 Community (with "Desktop development with C++")
- Qt 6.7.3, MSVC 2019 64-bit kit, at `C:\Qt\6.7.3\msvc2019_64` (with Qt WebEngine)
- CMake and Ninja (Ninja: `winget install Ninja-build.Ninja`)

**2. Get the code**
```text
git clone https://github.com/aaritdhingra/COREVEX.git
cd COREVEX
```

**3. Run it — double-click this file**
```text
start.bat
```

That is all. `start.bat` builds the app, copies the Qt files the app needs, and opens COREVEX. The first build takes a few minutes. If the build fails, the window stays open and shows the error.

**If you want to run it step by step instead:**
```text
build_core.bat
C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe build\COREVEX.exe
.\build\COREVEX.exe
```
`BUILD_OK` means the build worked. After that, `build\COREVEX.exe` runs the app any time.

**Quick check (no Qt needed):**
```text
test_ads.bat
```
This runs the data-structure tests only. `ALL ADS TESTS PASSED` (19/19) means the DSA library is working.

## What it does

- **Browser:** real multi-tab browsing, back / forward / reload, address bar that tells a website address from a search, bookmarks, history, downloads, and session restore (tabs reopen after closing the app).
- **Search, without a key:** the search page is built by COREVEX itself. Results are fetched, parsed and shown in the COREVEX theme, so there is no search API key and no bot-check page. Brave / Mojeek are tried first, Bing is the fallback, Google can be selected in Settings.
- **AI Overview:** a short summary made from the top search results, with source links. It is an extractive summary, not a generated answer.
- **AI sidebar:** chat with three providers — Local Ollama (default, `http://127.0.0.1:11434`), an OpenAI-compatible API, or Gemini. Add your own key in Settings. Without Ollama running and without a key, chat shows an error instead of a fake answer.
- **ADS Lab:** a page inside the app that demonstrates every data structure with live numbers.

## Data structures used

| Structure | Where it is used | Why this one |
| --- | --- | --- |
| Hash Table | Dictionary of the inverted index | Average O(1) lookup |
| Trie | Address-bar and search autocomplete | Prefix search in O(word length) |
| Binary Heap, Top-K | Picking the top 10 ranked results | O(n log K), no full sort needed |
| AVL Tree | Sorted data, range queries | Always balanced, O(log n) |
| Inverted Index | Full-text search over fetched pages | Word to pages, with positions for phrase search |
| Evidence Graph | AI Overview sources and claims | Shows which source supports or contradicts a claim |
| BM25 Ranking | Scoring search results | Term frequency, rare words, and document length, with k1 = 1.5 and b = 0.75 |

Benchmarks (5,000 documents, seed 42, median of 5 runs): index lookup 0.001 ms vs linear scan 0.037 ms; Heap Top-K 0.138 ms vs full sort 0.504 ms.

## Tech stack

- **Language:** C++20
- **Framework:** Qt 6.7.3, Qt Widgets, Qt WebEngine (Chromium)
- **Qt modules:** Core, Gui, Widgets, Network, Sql, WebEngineCore, WebEngineWidgets
- **Build:** CMake 3.25+, Ninja, MSVC 2022
- **Storage:** SQLite (`%LOCALAPPDATA%\COREVEX\browser\corevex.db` for bookmarks and history), Windows Registry via QSettings (settings, session, AI chat), WebEngine folder (cookies and cache). All data stays on the local machine. The data structures themselves live only in memory.
- **Platform:** Windows 10/11 x64 only. Qt WebEngine does not work with MinGW, so MSVC is required.

## Project layout

```text
CMakeLists.txt        Build file (Qt 6, C++20)
build_core.bat        Builds the app (prints BUILD_OK)
start.bat             Builds, deploys Qt files, and runs the app
test_ads.bat          Runs the 19 data-structure tests
src/
  app/                Main window, tabs, address bar
  ads/                Hash Table, Trie, Heap, AVL, Inverted Index, Graph
  ranking/            BM25 ranker
  search/             Custom search page, search engines
  ai/                 AI client (Ollama / OpenAI-compatible / Gemini)
  storage/            SQLite store (bookmarks, history)
  ui/                 New Tab, pages, ADS Lab, AI settings
  utilities/          Assets, URL handling, theme injection
assets/               Icons, backgrounds, logo
tests/unit/           Data-structure unit tests
```

## Honest limits

- No private / incognito tabs, no sync across devices, no browser extensions.
- Google may still show its own bot-check page if Google is selected as the search engine; the app then falls back to Bing.
- AI chat needs Ollama running locally or an API key added in Settings.
- The lava background image is not in this repo (it is a large binary file). The app still runs with its built-in background. To get the lava background back, put a `bg.png` in your Downloads folder or in `assets/backgrounds/` and restart the app.
