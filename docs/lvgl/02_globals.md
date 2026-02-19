# 02 — Globals

`globals.xml` defines project-wide shared resources. All sections are optional and can appear in any order.

```xml
<globals>
    <api>      <!-- global enums --></api>
    <consts>   <!-- named constants --></consts>
    <styles>   <!-- named styles --></styles>
    <subjects> <!-- reactive data subjects --></subjects>
    <images>   <!-- image assets --></images>
    <fonts>    <!-- font assets --></fonts>
</globals>
```

---

## `<consts>` — Named Constants

Reference in XML with the `#name` prefix.

```xml
<consts>
    <px     name="btn_w"       value="120"/>
    <px     name="btn_h"       value="44"/>
    <int    name="corner_r"    value="8"/>
    <color  name="primary"     value="0x1E88E5"/>
    <color  name="on_primary"  value="0xFFFFFF"/>
    <color  name="surface"     value="0x1a1a2e"/>
    <color  name="error"       value="0xCF6679"/>
    <string name="app_version" value="1.0.0"/>
</consts>
```

Usage:

```xml
<lv_button width="#btn_w" height="#btn_h"
           style_bg_color="#primary"
           style_text_color="#on_primary"
           style_radius="#corner_r"/>
```

### Constant Types

| Tag | C equivalent | Example |
|-----|-------------|---------|
| `<px>` | `lv_coord_t` | `value="120"` |
| `<int>` | `int32_t` | `value="300"` |
| `<color>` | `lv_color_t` | `value="0xFF5722"` |
| `<string>` | `const char*` | `value="Hello"` |

---

## `<styles>` — Named Styles

Define reusable style bundles. Reference via `styles="name"` attribute or `<style name="name"/>` child element.

```xml
<styles>
    <!-- Card surface -->
    <style name="card"
           bg_color="0x1e1e2e"
           radius="12"
           pad_all="16"
           shadow_width="8"
           shadow_color="0x000000"
           shadow_opa="80"/>

    <!-- Primary action button -->
    <style name="btn_primary"
           bg_color="0x1E88E5"
           radius="6"
           text_color="0xFFFFFF"
           pad_hor="16"
           pad_ver="8"/>

    <!-- Pressed state override -->
    <style name="btn_primary_pressed"
           bg_color="0x1565C0"/>

    <!-- Danger / destructive action -->
    <style name="btn_danger"
           bg_color="0xCF6679"
           text_color="0xFFFFFF"
           radius="6"
           pad_hor="16"
           pad_ver="8"/>
</styles>
```

Usage:

```xml
<!-- Inline attribute -->
<lv_button styles="btn_primary">
    <lv_label text="Save"/>
</lv_button>

<!-- Child elements for state-specific styles -->
<lv_button>
    <style name="btn_primary"/>
    <style name="btn_primary_pressed" selector="pressed"/>
    <lv_label text="Save"/>
</lv_button>
```

---

## `<subjects>` — Reactive Data Subjects

Observable values that drive live UI updates. See [07 — Data Binding](07_bindings.md) for full usage.

```xml
<subjects>
    <int    name="temperature" value="20"/>
    <int    name="humidity"    value="55"/>
    <float  name="voltage"     value="3.3"/>
    <string name="status"      value="Idle"/>
    <int    name="active_page" value="0"/>
    <int    name="error_code"  value="0"/>
</subjects>
```

Update from C — UI reacts automatically:

```c
lv_subject_set_int(&temperature_subject, 25);
```

To wire up a C-side subject declared in `globals.xml`:

```c
lv_subject_t temperature_subject;
lv_subject_init_int(&temperature_subject, 20);
lv_xml_register_subject(NULL, "temperature", &temperature_subject);
```

> Register subjects **before** calling `lv_xml_load_all_from_path()`.

---

## `<images>` — Image Assets

```xml
<images>
    <!-- File on LVGL file-system -->
    <file name="logo"          src="images/logo.png"/>
    <file name="home_icon"     src="images/home.png"/>
    <file name="settings_icon" src="images/settings.png"/>

    <!-- C variable (compiled-in image descriptor) -->
    <data name="icon_ok"  src="&amp;icon_ok_img_dsc"/>
    <data name="icon_err" src="&amp;icon_err_img_dsc"/>
</images>
```

Reference in XML: `<lv_image src="logo"/>` or `<lv_image src="home_icon"/>`.

Register a C variable image:

```c
LV_IMG_DECLARE(icon_ok_img_dsc);
lv_xml_register_image(NULL, "icon_ok", &icon_ok_img_dsc);
```

---

## `<fonts>` — Font Assets

```xml
<fonts>
    <!-- Pre-compiled .bin font -->
    <bin      name="font_small" src="fonts/montserrat_12.bin"/>
    <bin      name="font_ui"    src="fonts/montserrat_16.bin"/>
    <bin      name="font_big"   src="fonts/montserrat_24.bin"/>

    <!-- TrueType font via TinyTTF (requires LV_USE_TINY_TTF=1) -->
    <tiny_ttf name="font_body"  src="fonts/roboto.ttf" size="16"/>
    <tiny_ttf name="font_title" src="fonts/roboto_bold.ttf" size="20"/>
</fonts>
```

Reference in style: `style_text_font="font_ui"` or `text_font="font_ui"` in a style block.

Register a built-in/compiled font:

```c
lv_xml_register_font(NULL, "font_ui", &lv_font_montserrat_16);
```

---

## `<api>` — Global Enums

Enums defined in `<globals><api>` are available in every screen and component.

```xml
<api>
    <enumdef name="app_theme">
        <enum name="light" help="Light theme"/>
        <enum name="dark"  help="Dark theme"/>
    </enumdef>

    <enumdef name="alert_level">
        <enum name="info"    help="Informational"/>
        <enum name="warning" help="Warning"/>
        <enum name="error"   help="Error"/>
    </enumdef>
</api>
```

Use with component props:

```xml
<prop name="theme" type="enum:app_theme" default="dark"/>
```

---

## Complete globals.xml Template

```xml
<globals>
    <api>
        <enumdef name="page_id">
            <enum name="home"/>
            <enum name="settings"/>
            <enum name="about"/>
        </enumdef>
    </api>

    <consts>
        <px    name="screen_w"   value="320"/>
        <px    name="screen_h"   value="240"/>
        <px    name="btn_w"      value="120"/>
        <px    name="btn_h"      value="44"/>
        <int   name="corner_r"   value="8"/>
        <color name="primary"    value="0x1E88E5"/>
        <color name="surface"    value="0x1a1a2e"/>
        <color name="on_surface" value="0xFFFFFF"/>
        <color name="error_col"  value="0xCF6679"/>
    </consts>

    <styles>
        <style name="screen_bg"
               bg_color="#surface"
               bg_opa="255"/>
        <style name="card"
               bg_color="0x252536"
               radius="#corner_r"
               pad_all="12"
               shadow_width="6"
               shadow_color="0x000000"
               shadow_opa="60"/>
        <style name="btn_primary"
               bg_color="#primary"
               radius="6"
               text_color="#on_surface"
               pad_hor="16"
               pad_ver="8"/>
    </styles>

    <subjects>
        <int    name="active_page"  value="0"/>
        <int    name="temperature"  value="0"/>
        <float  name="humidity"     value="0.0"/>
        <int    name="error_code"   value="0"/>
        <string name="status_msg"   value="Ready"/>
    </subjects>

    <images>
        <file name="logo"          src="images/logo.png"/>
        <file name="home_icon"     src="images/ic_home.png"/>
        <file name="settings_icon" src="images/ic_settings.png"/>
    </images>

    <fonts>
        <bin name="font_small" src="fonts/montserrat_12.bin"/>
        <bin name="font_ui"    src="fonts/montserrat_16.bin"/>
        <bin name="font_big"   src="fonts/montserrat_24.bin"/>
    </fonts>
</globals>
```
