# LVGL XML Documentation

Complete reference and tutorial guide for LVGL v9.4+ XML-based UI.

## Sections

| File | Contents |
|------|----------|
| [01 — Getting Started](01_getting_started.md) | Enabling XML, file types, directory layout, project.xml |
| [02 — Globals](02_globals.md) | Shared constants, styles, subjects, images, fonts |
| [03 — Screens](03_screens.md) | Full-screen views: structure, loading, navigation |
| [04 — Components](04_components.md) | Reusable widget trees with typed APIs |
| [05 — Widgets](05_widgets.md) | Every built-in widget and its attributes |
| [06 — Styles](06_styles.md) | Inline props, named styles, selectors, gradients |
| [07 — Data Binding](07_bindings.md) | Subjects, reactive bindings, flag/state control |
| [08 — Events](08_events.md) | Callbacks, subject mutations, screen navigation |
| [09 — Layouts](09_layouts.md) | Flex and Grid layout systems |
| [10 — C API](10_c_api.md) | C integration, registration helpers, CMake setup |
| [11 — Examples](11_examples.md) | Full worked project walkthroughs |

## Quick-Start Checklist

1. Set `#define LV_USE_XML 1` in `lv_conf.h`
2. Call `lv_xml_init()` after `lv_init()`
3. Register resources (subjects, callbacks, fonts, images) **before** loading XML
4. Call `lv_xml_load_all_from_path("ui/")` to parse the whole tree
5. Create and load a screen: `lv_screen_load(lv_xml_create_screen("main"))`

## Key Syntax at a Glance

```xml
<!-- Inline style -->
<lv_button style_bg_color="0x1E88E5" style_radius="8"/>

<!-- Named style -->
<lv_button styles="primary_btn"/>

<!-- Component parameter -->
<my_card title="$title" color="$accent"/>

<!-- Named constant -->
<lv_button width="#btn_w" height="#btn_h"/>

<!-- Subject binding -->
<lv_label bind_text="temperature" fmt="%.1f °C"/>

<!-- Event callback -->
<lv_button>
    <event_cb callback="on_save" trigger="clicked"/>
</lv_button>
```
