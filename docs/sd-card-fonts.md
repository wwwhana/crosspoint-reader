# SD Card Fonts

CrossPoint supports loading additional fonts from the SD card, including fonts
with extended Unicode coverage (CJK, Cyrillic, Greek, etc.).

If CrossPoint enables external RAM on your device, you can load `.ttf`, `.otf`,
and `.ttc` files directly. Otherwise, convert them to `.cpfont` files first.
All devices can use `.cpfont` files.

## Installing Fonts

There are three ways to install `.cpfont` fonts. Direct TTF/OTF/TTC loading uses
the manual SD card copy method below.

### Option 1: Download from device (recommended)

1. Connect your CrossPoint reader to Wi-Fi
2. Go to **Settings > Reader > Manage Fonts**
3. Browse available font families and tap to download
4. Downloaded fonts appear immediately in **Settings > Reader > Font Family**

### Option 2: Upload via web browser

1. Start **File Transfer** and connect through **Join Network** or **Create Hotspot**
2. Open the web interface URL shown on the reader
3. Navigate to the **Fonts** tab
4. Upload `.cpfont` files using the upload form

### Option 3: Manual SD card copy

1. Download font files from the
   [crosspoint-fonts repository](https://github.com/crosspoint-reader/crosspoint-fonts)
2. Copy font family folders to one of two locations on your SD card:

   - `/.fonts/` — hidden directory (preferred; keeps the SD root tidy
     when mounted on a desktop)
   - `/fonts/` — visible directory (use this if your OS hides dot-files
     and you'd rather see the folder in your file manager)

   Both roots are always scanned at boot and the results are merged: a
   family installed in `/fonts/` shows up even when `/.fonts/` also
   exists, and vice versa. The two roots only collide if the same family
   name appears in both — in that case the copy in `/.fonts/` wins and
   the duplicate in `/fonts/` is ignored.

       SD Card Root/
       ├── .fonts/                     ← Hidden root (preferred)
       │   └── Literata/
       │       ├── Literata_12.cpfont
       │       ├── Literata_14.cpfont
       │       ├── Literata_16.cpfont
       │       └── Literata_18.cpfont
       └── fonts/                      ← Visible root (equally valid)
           └── Merriweather/
               ├── Merriweather_12.cpfont
               └── ...

3. Insert the SD card and power on your CrossPoint reader

### Direct TTF/OTF/TTC fonts

If CrossPoint enables external RAM on your device, copy a `.ttf`, `.otf`, or
`.ttc` file directly into `/fonts/` or `/.fonts/`. The filename without its
extension becomes the family name. For example, `/fonts/Bookerly.ttf` appears as
`Bookerly` in **Settings > Reader > Font Family**.

For a family with separate styles, put its files in one subfolder. The folder
name becomes the family name. For example:

    /fonts/Bookerly/Bookerly-Regular.ttf
    /fonts/Bookerly/Bookerly-Bold.ttf
    /fonts/Bookerly/Bookerly-Italic.ttf
    /fonts/Bookerly/Bookerly-BoldItalic.ttf

The reader selects regular, bold, italic, and bold italic faces from the font
files. One regular file is enough; the reader derives missing styles. A `.cpfont`
file in the same family folder takes priority over TTF/OTF/TTC files, so keep
the two formats in separate folders. If both font roots contain the same family
name, the copy in `/.fonts/` takes priority.

Direct fonts use the standard reader sizes: 12, 14, 16, and 18 pt. The **Fonts**
tab in File Transfer accepts `.cpfont` files only. Copy direct fonts to the SD
card instead. To remove a loose font file, delete that file from the SD card.
The Fonts page can delete families stored in subfolders.

## SD Card Fonts in the User Interface

The built-in UI fonts cover Latin, Cyrillic, Hebrew letters and core Arabic,
but no Greek, CJK or Hangul. Without an SD-card font the interface (book titles in the
library, file names in the browser, list rows, headers) shows replacement boxes
for those scripts, even when book *content* renders correctly.

To avoid shipping every script's glyph set in flash, CrossPoint reuses the
SD-card font you selected under **Settings > Reader > Font Family**: once one is
selected, the interface is drawn with it too, Latin text included. This applies
to any family, not only non-Latin ones.

The UI fonts are **size-matched**. The built-in UI fonts render at 8 pt
(small/author lines), 10 pt (list rows) and 12 pt (book-cover titles, headers),
so CrossPoint loads your SD family at those sizes and maps each UI font to its
same-size SD font. A `.cpfont` family must contain files at sizes **8, 10 and
12** (in addition to the reader sizes 12–18) for the interface to use it. A UI
size the family does not ship stays on the built-in font at that size, which
shows replacement boxes for scripts the built-in font lacks — **the most common
reason non-Latin UI text shows boxes even though the book renders fine.**
Direct TTF/OTF/TTC families use the same file at all three UI sizes when
enough RAM is free; otherwise the interface stays on the built-in fonts.

For `.cpfont` families, **Settings > Reader > Font Size** lists every size the family ships,
so a family built at 8,10,12,14,16,18 offers all six as reading sizes — the UI
sizes are not hidden from the list. Reading at 8 pt is your call; if you would
rather not see the small sizes there, convert two families (one with the UI
sizes, one with only the reading sizes you want).

When converting your own font, include the UI sizes:

    python3 lib/EpdFont/scripts/fontconvert_sdcard.py \
      MyCJKFont-Regular.otf \
      --intervals cjk \
      --sizes 8,10,12,14,16,18 \
      --style regular \
      --name MyCJKFont \
      --output-dir ./MyCJKFont/

What this means in practice:

- One selection under **Settings > Reader > Font Family** sets both book
  content *and* the interface (see [Installing Fonts](#installing-fonts) and
  the interval presets for `.cpfont` under
  [Converting Custom Fonts](#converting-custom-fonts)).
- The choice is per *string*, not per glyph. A string with a character only
  the SD font has always uses the SD font. Otherwise a string with a character
  the SD font lacks but the built-in font has uses the built-in font, so a
  CJK-only family leaves Latin-only menu rows on the built-in font. Every
  other string, plain Latin included, uses the SD font.
- A `Mono` family sets the Latin menu text in its half/full widths too.
- A family without a bold face draws bold headings in its regular weight.
- If no SD font is selected (a built-in reading font is active), the built-in
  UI fonts are used and scripts they lack show boxes.
- The Korean UI translation and the Korean keyboard's key labels need a Hangul
  family (Pretendard, RIDIBatang, MaruBuri) selected as the reader font; without
  one the menus and the jamo keys show replacement boxes, though typing still
  works.

## Available Pre-Built Fonts

The current list of pre-built fonts is maintained in the
[crosspoint-fonts repository](https://github.com/crosspoint-reader/crosspoint-fonts).

## Converting Custom Fonts

To make `.cpfont` files for any device, convert your TrueType/OpenType fonts:

### Prerequisites

    pip install freetype-py fonttools

### Single font (one style)

    python3 lib/EpdFont/scripts/fontconvert_sdcard.py \
      MyFont-Regular.ttf \
      --intervals latin-ext \
      --sizes 12,14,16,18 \
      --style regular \
      --name MyFont \
      --output-dir ./MyFont/

### Multi-style font

    python3 lib/EpdFont/scripts/fontconvert_sdcard.py \
      --regular MyFont-Regular.ttf \
      --bold MyFont-Bold.ttf \
      --italic MyFont-Italic.ttf \
      --bolditalic MyFont-BoldItalic.ttf \
      --intervals latin-ext \
      --sizes 12,14,16,18 \
      --name MyFont \
      --output-dir ./MyFont/

> **Using this font for the interface too?** The examples above only include
> the reader sizes (12–18). For the library, headers, and file browser to use
> it (see [SD Card Fonts in the User Interface](#sd-card-fonts-in-the-user-interface)),
> add the UI sizes: `--sizes 8,10,12,14,16,18`.

### Available Unicode interval presets

| Preset | Coverage |
|--------|----------|
| `ascii` | U+0020–U+007E (Basic Latin) |
| `latin1` | U+0080–U+00FF (Latin-1 Supplement) |
| `latin-ext` | European languages (Latin + Extended-A/B + punctuation + currency + letterlike symbols such as ™ and № + minus sign + ligatures) |
| `greek` | Greek + Extended Greek |
| `cyrillic` | Cyrillic + Supplement |
| `hebrew` | Hebrew + Alphabetic Presentation Forms |
| `arabic` | Arabic + Supplement + Extended-A + Presentation Forms A/B (RTL, contextual shaping) |
| `georgian` | Georgian + Georgian Supplement |
| `armenian` | Armenian |
| `ethiopic` | Ethiopic + Extended |
| `vietnamese` | Vietnamese subset (ơ/ư and combining marks) |
| `ipa-chars` | IPA Extensions + Spacing Modifier Letters (phonetic transcription) |
| `punctuation` | General punctuation (U+2000–U+206F) |
| `cjk` | CJK Unified Ideographs + Hiragana + Katakana + Fullwidth |
| `hangul` | Korean Hangul syllables + Jamo + Compatibility Jamo |
| `cherokee` | Cherokee (historic + supplement block) |
| `tifinagh` | Tifinagh |
| `symbols` | Math, currency, arrows, box-drawing, misc symbols, dingbats |
| `reading` | Literary fiction coverage: Latin, Greek, Cyrillic, math/symbol blocks, supplemental punctuation, and CJK quote marks |
| `builtin` | Matches the firmware's built-in font conversion intervals |

Combine presets with commas: `--intervals latin-ext,greek,cyrillic`

You can also specify arbitrary Unicode ranges directly:
`--intervals 'latin-ext,(0x2190-0x21FF)'`

To list all presets with codepoint counts:

    python3 lib/EpdFont/scripts/fontconvert_sdcard.py --list-presets

### Additional options

`--force-autohint` — force FreeType's auto-hinter instead of the font's native hinting (useful when a font's built-in hints produce poor results at small sizes).

Install custom fonts via the web interface or manual SD card copy.
