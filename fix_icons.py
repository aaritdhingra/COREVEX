import os
icons = {
    "assets/icons/window_min.svg": '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round"><line x1="3" y1="8" x2="13" y2="8"/></svg>',
    "assets/icons/window_max.svg": '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round"><rect x="3.5" y="3.5" width="9" height="9" rx="1"/></svg>',
    "assets/icons/window_close.svg": '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="none" stroke="#A1A1AA" stroke-width="1.5" stroke-linecap="round"><line x1="4" y1="4" x2="12" y2="12"/><line x1="12" y1="4" x2="4" y2="12"/></svg>',
}
for p, c in icons.items():
    os.makedirs(os.path.dirname(p), exist_ok=True)
    open(p, "w", encoding="utf-8").write(c)
print("Window icons OK")
