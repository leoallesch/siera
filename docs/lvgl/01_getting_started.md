# 01 — Getting Started with LVGL XML

LVGL v9.4 ships an XML-based UI description system. You describe screens, components, and styles in `.xml` files; the LVGL parser instantiates the widget tree at runtime.

---

## Enabling XML Support

In `lv_conf.h`:

```c
#define LV_USE_XML 1
```

In your application's startup code, **after** `lv_init()` and display initialisation:

```c
lv_xml_init();
```

On shutdown:

```c
lv_xml_deinit();
```

---

## File Types

| File | Root tag | Purpose |
|------|----------|---------|
| `project.xml` | `<project>` | Display target metadata |
| `globals.xml` | `<globals>` | Shared styles, constants, subjects, assets |
| `<name>.xml` (screen) | `<screen>` | Full-screen views |
| `<name>.xml` (component) | `<component>` | Reusable widget trees |

---

## Recommended Directory Layout

```
ui/
├── project.xml
├── globals.xml
├── components/
│   ├── sensor_card.xml
│   ├── nav_button.xml
│   └── status_badge.xml
└── screens/
    ├── home.xml
    ├── settings.xml
    └── detail.xml
```

Load everything at once:

```c
lv_xml_load_all_from_path("A:ui/");   // 'A:' prefix for LVGL file-system
```

`lv_xml_load_all_from_path()` does multiple passes to resolve forward references, so the directory order does not matter.

---

## project.xml

Declares display targets (metadata only — does not create displays).

```xml
<project name="my_app">
    <targets>
        <target name="main_display">
            <display width="320" height="240"/>
        </target>
    </targets>
</project>
```

---

## Minimal "Hello, World" Walkthrough

### 1. `ui/globals.xml`

```xml
<globals>
    <consts>
        <color name="bg"      value="0x1a1a2e"/>
        <color name="fg"      value="0xFFFFFF"/>
    </consts>
</globals>
```

### 2. `ui/screens/home.xml`

```xml
<screen>
    <view style_bg_color="#bg" style_bg_opa="255">
        <lv_label text="Hello, World!"
                  style_text_color="#fg"
                  align="center"/>
    </view>
</screen>
```

### 3. C startup

```c
#include "lvgl/lvgl.h"

void app_start(void) {
    lv_xml_init();
    lv_xml_load_all_from_path("A:ui/");
    lv_obj_t *scr = lv_xml_create_screen("home");
    lv_screen_load(scr);
}
```

---

## Asset Path Prefix

If your LVGL file-system uses a drive letter, set the default prefix so all `src=` paths resolve correctly:

```c
lv_xml_set_default_asset_path("A:ui/");
```

After this, `<file name="logo" src="images/logo.png"/>` resolves to `A:ui/images/logo.png`.

---

## Loading Order Rules

1. `globals.xml` must be parsed first (constants, styles, subjects referenced by everything else).
2. Component XMLs must be registered before screens that use them as tags.
3. Screen XMLs come last.

`lv_xml_load_all_from_path()` handles this automatically. When registering manually, follow this order:

```c
lv_xml_init();

// 1. Register resources (subjects, callbacks, fonts, images)
lv_xml_register_subject(NULL, "temperature", &temp_subject);
lv_xml_register_event_cb(NULL, "on_btn_click", my_handler);
lv_xml_register_font(NULL, "font_ui", &lv_font_montserrat_16);

// 2. Load globals
lv_xml_register_component_from_file("ui/globals.xml");   // treats globals like a component

// 3. Load components
lv_xml_register_component_from_file("ui/components/sensor_card.xml");

// 4. Create screens
lv_obj_t *scr = lv_xml_create_screen("home");  // parsed on demand
lv_screen_load(scr);
```

---

## Next Steps

- [02 — Globals](02_globals.md) — constants, styles, subjects
- [03 — Screens](03_screens.md) — screen files and navigation
- [04 — Components](04_components.md) — reusable widget trees
