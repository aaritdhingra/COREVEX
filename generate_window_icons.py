import os
new_assets = {
    "assets/icons/window_min.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="none" stroke="#A1A1AA" stroke-width="1.5"><line x1="3" y1="8" x2="13" y2="8"/></svg>''',
    "assets/icons/window_max.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="none" stroke="#A1A1AA" stroke-width="1.5"><rect x="3" y="3" width="10" height="10" rx="1"/></svg>''',
    "assets/icons/window_close.svg": '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="none" stroke="#A1A1AA" stroke-width="1.5"><line x1="4" y1="4" x2="12" y2="12"/><line x1="12" y1="4" x2="4" y2="12"/></svg>'''
}
for path, code in new_assets.items():
    with open(path, "w", encoding="utf-8") as f:
        f.write(code.strip())
print("Added 3 window control SVGs!")
