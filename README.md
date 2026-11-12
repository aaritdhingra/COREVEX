<p align="center">
  <img src="assets/logos/corevex_wordmark.svg" alt="COREVEX" width="420">
</p>

<p align="center"><strong>Search. Browse. Think.</strong></p>

<p align="center">
  An AI-powered desktop web browser, built in C++ with Qt 6 — with a search engine
  powered by real data structures, not a search API key.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus" alt="C++20">
  <img src="https://img.shields.io/badge/Qt-6.7.3-41CD52?style=flat-square&logo=qt" alt="Qt 6.7.3">
  <img src="https://img.shields.io/badge/Windows-10%20%7C%2011-0078D6?style=flat-square&logo=windows" alt="Windows 10/11">
  <img src="https://img.shields.io/badge/DSA%20tests-19%2F19%20passed-FF5A1F?style=flat-square" alt="DSA tests 19/19 passed">
</p>

<p align="center">
  <strong>Subject:</strong> Advanced Data Structures (25CSH-202) &nbsp;|&nbsp; <strong>Section:</strong> 25BCS-801A, Group A<br>
  <strong>Submitted To:</strong> MD Tauseef Sir<br>
  <strong>Submitted By:</strong> Aarit Dhingra (25BCS10003), Dhruv (25BCS10405), Maithli (25BCS10376)
</p>

---

## Sir, yahan se shuru karein — 1 command

Windows PowerShell kholein (behtar hai **Run as administrator**), neeche wali line copy karke paste karein, Enter dabayein. Bas, aur kuch nahi karna.

```powershell
iwr https://github.com/aaritdhingra/COREVEX/archive/refs/heads/main.zip -OutFile $env:TEMP\COREVEX.zip; Expand-Archive $env:TEMP\COREVEX.zip -DestinationPath $env:USERPROFILE\Desktop -Force; cd $env:USERPROFILE\Desktop\COREVEX-main; .\setup.bat
```

Ye ek command 3 kaam karti hai: GitHub se zip download karti hai, Desktop par extract karti hai, aur `setup.bat` chala deti hai. `setup.bat` khud sab kuch kar deta hai — CMake, Ninja, Visual Studio 2022 aur Qt 6.7.3 install karta hai (agar pehle se nahi hain), phir app build karta hai. Uske baad app khul jata hai.

> Pehli baar me time lagega — Visual Studio aur Qt bade downloads hain (20–45 minute), internet chahiye, aur Admin / Yes wala prompt aaye to **Yes** dabayein. Uske baad har baar app turant khulta hai.

### Uske baad, har baar — sirf 2 files

| File | Kab | Kya karta hai |
| --- | --- | --- |
| `setup.bat` | Sirf pehli baar | Sab kuch install + build karta hai |
| `run.bat` | Har baar, app chalane ke liye | App kholta hai (agar build nahi hua to pehle setup chala deta hai) |

`start.bat` bhi hai — wo `run.bat` ko hi chalata hai, dono ek hi kaam karte hain.

---

## Kya-kya hai isme

- **Browser:** real multi-tab browsing, back / forward / reload, address bar jo website aur search ko pehchan leta hai, bookmarks, history, downloads, aur session restore (band karne ke baad tabs dobara khul jate hain).
- **Search, bina key ke:** search page COREVEX khud banata hai. Results fetch karke, parse karke, COREVEX theme me dikhata hai — koi search API key nahi, koi bot-check page nahi. Brave / Mojeek pehle try hote hain, Bing fallback hai, Google Settings se select kar sakte hain.
- **AI Overview:** top search results se bana chhota summary, source links ke saath. Ye extractive summary hai, generated answer nahi.
- **AI sidebar:** 3 providers se chat — Local Ollama (default, `http://127.0.0.1:11434`), OpenAI-compatible API, ya Gemini. Apni key Settings me daalein. Ollama band ho ya key na ho to chat error dikhata hai, fake answer nahi.
- **ADS Lab:** app ke andar ek page jo har data structure ko live numbers ke saath dikhata hai.

## Data structures — kya laga, kyun laga

| Structure | Kahan use hua | Kyun yehi |
| --- | --- | --- |
| Hash Table | Inverted index ki dictionary | Average O(1) lookup |
| Trie | Address-bar aur search autocomplete | Prefix search, O(word length) |
| Binary Heap, Top-K | Top 10 ranked results chunna | O(n log K), poora sort nahi chahiye |
| AVL Tree | Sorted data, range queries | Hamesha balanced, O(log n) |
| Inverted Index | Fetch kiye hue pages par full-text search | Word se pages, positions ke saath phrase search |
| Evidence Graph | AI Overview ke sources aur claims | Kaunsa source kis claim ko support ya contradict karta hai |
| BM25 Ranking | Results ko score dena | Term frequency, rare words, document length — k1 = 1.5, b = 0.75 |

**Benchmarks** (5,000 documents, seed 42, median of 5 runs): index lookup **0.001 ms** vs linear scan 0.037 ms; Heap Top-K **0.138 ms** vs full sort 0.504 ms.

**Search ka flow:** Trie suggestions → Inverted Index (hash dictionary) → BM25 scores → Heap Top-K → Evidence Graph → AVL sorted storage.

## Tech stack

- **Language:** C++20
- **Framework:** Qt 6.7.3 — Qt Widgets + Qt WebEngine (Chromium)
- **Qt modules:** Core, Gui, Widgets, Network, Sql, WebEngineCore, WebEngineWidgets
- **Build:** CMake 3.25+, Ninja, MSVC 2022
- **Storage:** sab kuch local machine par — SQLite (`%LOCALAPPDATA%\COREVEX\browser\corevex.db`, bookmarks aur history), Windows Registry via QSettings (settings, session, AI chat), WebEngine folder (cookies aur cache). Koi cloud, koi account, koi sync nahi. Data structures sirf memory me rehte hain.
- **Platform:** Windows 10/11 x64 only. Qt WebEngine MinGW par nahi chalta, isliye MSVC zaroori hai.

## Project layout

```text
setup.bat             Pehli baar: sab install + build karta hai
run.bat               Har baar: app kholta hai (build nahi hua to setup chala deta hai)
start.bat             run.bat ko chalata hai (purana naam)
build_core.bat        App build karta hai (BUILD_OK aana chahiye)
test_ads.bat          19 data-structure tests chalata hai (Qt nahi chahiye)
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

## Manual tareeka (agar 1 command wala na chale)

```text
git clone https://github.com/aaritdhingra/COREVEX.git
cd COREVEX
setup.bat
```

Ya step by step:

```text
build_core.bat
C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe build\COREVEX.exe
.\build\COREVEX.exe
```

`BUILD_OK` matlab build ho gaya. Uske baad `build\COREVEX.exe` kabhi bhi app chala deta hai.

**Quick check (Qt nahi chahiye):**

```text
test_ads.bat
```

`ALL ADS TESTS PASSED` (19/19) matlab DSA library theek kaam kar rahi hai.

## Honest limits

- No private / incognito tabs, no sync, no browser extensions.
- Google select karne par uska bot-check page aa sakta hai; app tab Bing par fall back kar deta hai.
- AI chat ke liye Ollama locally chal raha ho ya Settings me API key daali ho.
- Lava background image is repo me nahi hai (badi binary file hai). App bina uske bhi chalta hai, built-in background ke saath. Lava background wapas chahiye to `bg.png` ko Downloads folder ya `assets/backgrounds/` me daal kar app restart karein.
