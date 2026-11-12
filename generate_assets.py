import os

assets = {
    # LOGOS
    "assets/logos/corevex_logo.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
  <defs>
    <linearGradient id="og" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FF5A1F"/>
      <stop offset="100%" stop-color="#F02D24"/>
    </linearGradient>
  </defs>
  <rect width="64" height="64" rx="16" fill="#1B1E24"/>
  <polygon points="32,8 52,19.5 52,44.5 32,56 12,44.5 12,19.5" fill="none" stroke="url(#og)" stroke-width="3"/>
  <path d="M40 22 C34 16, 20 20, 20 32 C20 44, 34 48, 40 42" fill="none" stroke="url(#og)" stroke-width="4.5" stroke-linecap="round"/>
</svg>''',

    "assets/logos/corevex_wordmark.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 180 40" width="180" height="40">
  <defs>
    <linearGradient id="ogw" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FF5A1F"/>
      <stop offset="100%" stop-color="#F02D24"/>
    </linearGradient>
  </defs>
  <g transform="translate(0, 4)">
    <polygon points="16,2 28,9 28,23 16,30 4,23 4,9" fill="none" stroke="url(#ogw)" stroke-width="2"/>
    <path d="M21 11 C17 7, 9 10, 9 16 C9 22, 17 25, 21 21" fill="none" stroke="url(#ogw)" stroke-width="2.5" stroke-linecap="round"/>
  </g>
  <text x="38" y="26" font-family="'Segoe UI', sans-serif" font-weight="800" font-size="20" fill="#F4F4F5" letter-spacing="2">COREVEX</text>
</svg>''',

    "assets/logos/corevex_watermark.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 220 120" width="220" height="120">
  <defs>
    <linearGradient id="ogwm" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FF5A1F"/>
      <stop offset="100%" stop-color="#F02D24"/>
    </linearGradient>
  </defs>
  <rect width="220" height="120" rx="12" fill="#1B1E24" stroke="#2A2D35" stroke-width="1"/>
  <path d="M120 10 Q160 60 220 40 T220 110" fill="none" stroke="url(#ogwm)" stroke-width="1.5" opacity="0.3"/>
  <polygon points="30,20 44,28 44,44 30,52 16,44 16,28" fill="none" stroke="url(#ogwm)" stroke-width="2"/>
  <path d="M35 29 C31 25, 23 28, 23 36 C23 42, 31 45, 35 41" fill="none" stroke="url(#ogwm)" stroke-width="2.5" stroke-linecap="round"/>
  <text x="52" y="42" font-family="'Segoe UI', sans-serif" font-weight="800" font-size="16" fill="#F4F4F5" letter-spacing="1.5">COREVEX</text>
  <text x="16" y="80" font-family="'Segoe UI', sans-serif" font-weight="600" font-size="12" fill="#A1A1AA">Search. Browse. <tspan fill="#FF5A1F">Think.</tspan></text>
  <text x="16" y="100" font-family="'Segoe UI', sans-serif" font-weight="400" font-size="10" fill="#6B7280">AI-Native Engine v1.0</text>
</svg>''',

    # NAV ICONS
    "assets/icons/nav_browser.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="16" rx="3"/><line x1="3" y1="9" x2="21" y2="9"/><circle cx="6" cy="6.5" r="0.75" fill="#A1A1AA"/><circle cx="8.5" cy="6.5" r="0.75" fill="#A1A1AA"/><circle cx="11" cy="6.5" r="0.75" fill="#A1A1AA"/></svg>''',
    "assets/icons/nav_search.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="7"/><line x1="21" y1="21" x2="16.05" y2="16.05"/></svg>''',
    "assets/icons/nav_ai.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#FF5A1F" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2L14.5 9.5L22 12L14.5 14.5L12 22L9.5 14.5L2 12L9.5 9.5L12 2Z"/><path d="M19 2L20.2 5.8L24 7L20.2 8.2L19 12L17.8 8.2L14 7L17.8 5.8L19 2Z" opacity="0.6"/></svg>''',
    "assets/icons/nav_library.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20"/><path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z"/><line x1="9" y1="7" x2="15" y2="7"/></svg>''',
    "assets/icons/nav_bookmarks.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M19 21l-7-5-7 5V5a2 2 0 0 1 2-2h10a2 2 0 0 1 2 2z"/></svg>''',
    "assets/icons/nav_history.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="9"/><polyline points="12 7 12 12 15 15"/><path d="M3.05 11a9 9 0 1 1 .5 4"/></svg>''',
    "assets/icons/nav_downloads.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>''',
    "assets/icons/nav_ads_lab.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="6" cy="6" r="3"/><circle cx="18" cy="6" r="3"/><circle cx="12" cy="18" r="3"/><line x1="8.5" y1="7.5" x2="15.5" y2="7.5"/><line x1="7.5" y1="8.5" x2="10.5" y2="15.5"/><line x1="16.5" y1="8.5" x2="13.5" y2="15.5"/></svg>''',
    "assets/icons/nav_settings.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>''',
    "assets/icons/chevron_down.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M4 6l4 4 4-4"/></svg>''',

    # CHROME ICONS
    "assets/icons/chrome_back.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" width="20" height="20" fill="none" stroke="#F4F4F5" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M12.5 15l-5-5 5-5"/></svg>''',
    "assets/icons/chrome_forward.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" width="20" height="20" fill="none" stroke="#6B7280" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M7.5 15l5-5-5-5"/></svg>''',
    "assets/icons/chrome_reload.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" width="20" height="20" fill="none" stroke="#F4F4F5" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M16 10a6 6 0 1 1-1.8-4.2L16 8"/><path d="M16 4v4h-4"/></svg>''',
    "assets/icons/chrome_home.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" width="20" height="20" fill="none" stroke="#F4F4F5" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M3 8.5L10 3l7 5.5V16a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1V8.5z"/><path d="M8 17v-6h4v6"/></svg>''',
    "assets/icons/chrome_search.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="8" cy="8" r="5"/><line x1="16" y1="16" x2="11.8" y2="11.8"/></svg>''',
    "assets/icons/chrome_bookmark.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><polygon points="9 2 11.2 6.5 16 7.2 12.5 10.6 13.3 15.5 9 13.2 4.7 15.5 5.5 10.6 2 7.2 6.8 6.5 9 2"/></svg>''',
    "assets/icons/chrome_download.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><path d="M15 12v3a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-3"/><polyline points="5 8 9 12 13 8"/><line x1="9" y1="12" x2="9" y2="2"/></svg>''',
    "assets/icons/chrome_settings.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.75" stroke-linecap="round" stroke-linejoin="round"><circle cx="9" cy="9" r="2.5"/><path d="M14.5 11a1.2 1.2 0 0 0 .25 1.3l.05.05a1.5 1.5 0 0 1-2.12 2.12l-.05-.05a1.2 1.2 0 0 0-1.3-.25 1.2 1.2 0 0 0-.75 1.1V15.5a1.5 1.5 0 0 1-3 0v-.23a1.2 1.2 0 0 0-.75-1.1 1.2 1.2 0 0 0-1.3.25l-.05.05a1.5 1.5 0 0 1-2.12-2.12l.05-.05a1.2 1.2 0 0 0 .25-1.3 1.2 1.2 0 0 0-1.1-.75H2.5a1.5 1.5 0 0 1 0-3h.23a1.2 1.2 0 0 0 1.1-.75 1.2 1.2 0 0 0-.25-1.3l-.05-.05a1.5 1.5 0 0 1 2.12-2.12l.05.05a1.2 1.2 0 0 0 1.3.25H7.1a1.2 1.2 0 0 0 .75-1.1V2.5a1.5 1.5 0 0 1 3 0v.23a1.2 1.2 0 0 0 .75 1.1 1.2 1.2 0 0 0 1.3-.25l.05-.05a1.5 1.5 0 0 1 2.12 2.12l-.05.05a1.2 1.2 0 0 0-.25 1.3v.8a1.2 1.2 0 0 0 1.1.75h.23a1.5 1.5 0 0 1 0 3h-.23a1.2 1.2 0 0 0-1.1.75z"/></svg>''',
    "assets/icons/chrome_tab_close.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 14 14" width="14" height="14" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round"><line x1="3" y1="3" x2="11" y2="11"/><line x1="11" y1="3" x2="3" y2="11"/></svg>''',
    "assets/icons/chrome_tab_plus.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16" fill="none" stroke="#F4F4F5" stroke-width="1.75" stroke-linecap="round"><line x1="8" y1="3" x2="8" y2="13"/><line x1="3" y1="8" x2="13" y2="8"/></svg>''',

    # AI SIDEBAR ICONS
    "assets/icons/ai_logo_mark.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 28 28" width="28" height="28">
  <defs>
    <linearGradient id="ailg" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FF5A1F"/>
      <stop offset="100%" stop-color="#F02D24"/>
    </linearGradient>
  </defs>
  <polygon points="14,2 25,8.3 25,20.7 14,27 3,20.7 3,8.3" fill="none" stroke="url(#ailg)" stroke-width="2"/>
  <path d="M18 10 C14 6, 8 9, 8 14 C8 19, 14 22, 18 18" fill="none" stroke="url(#ailg)" stroke-width="2.5" stroke-linecap="round"/>
</svg>''',
    "assets/icons/ai_research_badge.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 14 14" width="14" height="14" fill="none" stroke="#FF5A1F" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M4 2h6v5a3 3 0 0 1-6 0V2z"/><path d="M7 10v2"/><path d="M4 12h6"/></svg>''',
    "assets/icons/ai_delete.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><polyline points="2 4 4 4 16 4"/><path d="M14 4v11a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2V4m3 0V2a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2"/></svg>''',
    "assets/icons/ai_settings.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><circle cx="9" cy="9" r="2.5"/><path d="M14.5 11a1.2 1.2 0 0 0 .25 1.3l.05.05a1.5 1.5 0 0 1-2.12 2.12l-.05-.05a1.2 1.2 0 0 0-1.3-.25 1.2 1.2 0 0 0-.75 1.1V15.5a1.5 1.5 0 0 1-3 0v-.23a1.2 1.2 0 0 0-.75-1.1 1.2 1.2 0 0 0-1.3.25l-.05.05a1.5 1.5 0 0 1-2.12-2.12l.05-.05a1.2 1.2 0 0 0 .25-1.3 1.2 1.2 0 0 0-1.1-.75H2.5a1.5 1.5 0 0 1 0-3h.23a1.2 1.2 0 0 0 1.1-.75 1.2 1.2 0 0 0-.25-1.3l-.05-.05a1.5 1.5 0 0 1 2.12-2.12l.05.05a1.2 1.2 0 0 0 1.3.25H7.1a1.2 1.2 0 0 0 .75-1.1V2.5a1.5 1.5 0 0 1 3 0v.23a1.2 1.2 0 0 0 .75 1.1 1.2 1.2 0 0 0 1.3-.25l.05-.05a1.5 1.5 0 0 1 2.12 2.12l-.05.05a1.2 1.2 0 0 0-.25 1.3v.8a1.2 1.2 0 0 0 1.1.75h.23a1.5 1.5 0 0 1 0 3h-.23a1.2 1.2 0 0 0-1.1.75z"/></svg>''',
    "assets/icons/ai_expand.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="3" width="14" height="12" rx="2"/><line x1="11" y1="3" x2="11" y2="15"/></svg>''',
    "assets/icons/ai_attach.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><path d="M16 8.5l-6.8 6.8a4 4 0 0 1-5.7-5.7l7.8-7.8a2.6 2.6 0 0 1 3.7 3.7l-7.8 7.8a1.3 1.3 0 0 1-1.8-1.8l6.8-6.8"/></svg>''',
    "assets/icons/ai_send.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18" width="18" height="18" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="4" y1="14" x2="14" y2="4"/><polyline points="6 4 14 4 14 12"/></svg>''',
    "assets/icons/ai_user_avatar.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 28 28" width="28" height="28"><circle cx="14" cy="14" r="14" fill="#2A2D35"/><circle cx="14" cy="10" r="5" fill="#A1A1AA"/><path d="M5 24 C5 18, 9 17, 14 17 C19 17, 23 18, 23 24" fill="#A1A1AA"/></svg>''',
    "assets/icons/ai_checkmark.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 14 14" width="14" height="14" fill="none" stroke="#22C55E" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="2.5 7.5 5.5 10.5 11.5 3.5"/></svg>''',
    "assets/icons/ai_sparkle.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16">
  <defs>
    <linearGradient id="spk" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FF5A1F"/>
      <stop offset="100%" stop-color="#F02D24"/>
    </linearGradient>
  </defs>
  <path d="M8 1L9.8 6.2L15 8L9.8 9.8L8 15L6.2 9.8L1 8L6.2 6.2L8 1Z" fill="url(#spk)"/>
</svg>''',

    # QUICK TILES
    "assets/icons/quick/quick_github.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32"><rect width="32" height="32" rx="8" fill="#181717"/><path d="M16 6a10 10 0 0 0-3.16 19.49c.5.09.68-.22.68-.48v-1.7c-2.78.6-3.37-1.34-3.37-1.34-.46-1.16-1.11-1.47-1.11-1.47-.91-.62.07-.6.07-.6 1 .07 1.53 1.03 1.53 1.03.89 1.52 2.34 1.08 2.91.83.09-.65.35-1.09.63-1.34-2.22-.25-4.55-1.11-4.55-4.94 0-1.09.39-1.98 1.03-2.68-.1-.25-.45-1.27.1-2.64 0 0 .84-.27 2.75 1.02a9.58 9.58 0 0 1 5 0c1.91-1.29 2.75-1.02 2.75-1.02.55 1.37.2 2.39.1 2.64.64.7 1.03 1.59 1.03 2.68 0 3.84-2.34 4.68-4.57 4.93.36.31.68.92.68 1.85v2.75c0 .27.18.58.69.48A10 10 0 0 0 16 6z" fill="#FFF"/></svg>''',
    "assets/icons/quick/quick_youtube.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32"><rect width="32" height="32" rx="8" fill="#282828"/><path d="M25.5 10.8a2.5 2.5 0 0 0-1.76-1.76C22.18 8.5 16 8.5 16 8.5s-6.18 0-7.74.54A2.5 2.5 0 0 0 6.5 10.8C6 12.36 6 16 6 16s0 3.64.5 5.2a2.5 2.5 0 0 0 1.76 1.76c1.56.54 7.74.54 7.74.54s6.18 0 7.74-.54a2.5 2.5 0 0 0 1.76-1.76C26 19.64 26 16 26 16s0-3.64-.5-5.2z" fill="#FF0000"/><polygon points="14,12 20,16 14,20" fill="#FFF"/></svg>''',
    "assets/icons/quick/quick_leetcode.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32"><rect width="32" height="32" rx="8" fill="#282828"/><path d="M18.5 8l-8 7.5a2 2 0 0 0 0 2.8l8 7.5" fill="none" stroke="#FFA116" stroke-width="3" stroke-linecap="round"/><path d="M12 16h12" fill="none" stroke="#FFF" stroke-width="3" stroke-linecap="round"/></svg>''',
    "assets/icons/quick/quick_google.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32"><rect width="32" height="32" rx="8" fill="#282828"/><path d="M23.5 16.2c0-.6-.05-1.2-.15-1.7H16v3.2h4.2a3.6 3.6 0 0 1-1.6 2.4v2h2.6c1.5-1.4 2.3-3.5 2.3-5.9z" fill="#4285F4"/><path d="M16 23.8c2.1 0 3.9-.7 5.2-1.9l-2.6-2c-.7.5-1.6.8-2.6.8-2 0-3.7-1.4-4.3-3.2H9.1v2c1.3 2.6 4 4.3 6.9 4.3z" fill="#34A853"/><path d="M11.7 16.9a4.8 4.8 0 0 1 0-1.8v-2H9.1a8 8 0 0 0 0 5.8l2.6-2z" fill="#FBBC05"/><path d="M16 10.2c1.1 0 2.2.4 3 1.1l2.3-2.3C19.9 7.7 18.1 7 16 7c-2.9 0-5.6 1.7-6.9 4.3l2.6 2c.6-1.8 2.3-3.1 4.3-3.1z" fill="#EA4335"/></svg>''',
    "assets/icons/quick/quick_chatgpt.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32"><rect width="32" height="32" rx="8" fill="#10A37F"/><circle cx="16" cy="16" r="8" fill="none" stroke="#FFF" stroke-width="2.5" stroke-dasharray="4 2"/></svg>''',
    "assets/icons/quick/quick_linkedin.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32"><rect width="32" height="32" rx="8" fill="#0A66C2"/><path d="M10 12h3v10h-3V12zm1.5-4a1.75 1.75 0 1 1 0 3.5 1.75 1.75 0 0 1 0-3.5zM15 12h2.8v1.4h.1c.4-.7 1.3-1.5 2.7-1.5 2.9 0 3.4 1.9 3.4 4.4V22h-3v-5c0-1.2 0-2.7-1.7-2.7-1.7 0-1.9 1.3-1.9 2.6V22h-3V12z" fill="#FFF"/></svg>''',

    # HERO BACKGROUND
    "assets/backgrounds/hero_waves.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 600" width="1200" height="600">
  <defs>
    <linearGradient id="hwg1" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FF5A1F" stop-opacity="0.25"/>
      <stop offset="100%" stop-color="#F02D24" stop-opacity="0.05"/>
    </linearGradient>
    <linearGradient id="hwg2" x1="100%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#FF5A1F" stop-opacity="0.4"/>
      <stop offset="100%" stop-color="#0B0C0F" stop-opacity="0"/>
    </linearGradient>
  </defs>
  <rect width="1200" height="600" fill="#0B0C0F"/>
  <path d="M700 0 C850 150, 1000 100, 1200 300 L1200 0 Z" fill="url(#hwg1)"/>
  <path d="M600 600 C750 350, 950 450, 1200 600 Z" fill="url(#hwg1)"/>
  <path d="M750 0 Q950 300 1200 150" fill="none" stroke="url(#hwg2)" stroke-width="2"/>
  <path d="M650 600 Q900 200 1200 400" fill="none" stroke="url(#hwg2)" stroke-width="1.5"/>
  <path d="M800 200 Q1000 500 1200 300" fill="none" stroke="#FF5A1F" stroke-width="0.75" opacity="0.3"/>
</svg>'''
}

for path, code in assets.items():
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(code.strip())

print("SUCCESS: All 40 SVGs generated in local assets subfolders!")
