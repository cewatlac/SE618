# SE618 — Course Materials Website

A ready-to-publish **public GitHub Pages website** for **Engineering Data Structures for Scalable Software Systems** (MSc Software Engineering, FCAI, Cairo University).

## Contents

- `index.html` — public homepage, search, topic filters and all 14 lectures
- `viewer.html` — online preview for Markdown, C++ and text student files
- `Lecture_01/` through `Lecture_14/` — interactive slide decks and their student resources
- `common/` — shared lecture slide system
- `assets/` — site styling, JavaScript and lecture catalog
- `downloads/` — individual lecture resource ZIPs + `SE618_All_Student_Resources.zip`

The public slide decks are **student-safe copies**: all instructor-only `<aside class="notes">` sections have been removed from the HTML before packaging. Never commit the private originals if you want notes or model answers to remain private.

## Publishing to GitHub Pages (no coding needed)

1. Create a new **Public** repository at GitHub (for example, `SE618-Course-Materials`).
2. Extract the provided website ZIP on your computer.
3. Upload the **contents** of the extracted folder, including `index.html` at the repository root. **Do not upload only the ZIP** or nest all files one folder deeper. GitHub permits at most **100 files in one browser upload** and this project has **105 files**, so either upload in two batches using **Add file → Upload files**, or use **GitHub Desktop** to commit and push the whole folder in one step.
4. Commit your changes (or, using GitHub Desktop, click **Commit to main → Push origin**).
5. Go to **Settings → Pages**. Under **Build and deployment**, set **Source: Deploy from a branch**, then select **Branch: main**, **Folder: / (root)** and **Save**.
6. After GitHub finishes deploying, your public website URL is generally:
   `https://YOUR-USERNAME.github.io/SE618-Course-Materials/`

You can also run locally using a static HTTP server, e.g. `python -m http.server 8000`, and browse to `http://localhost:8000/`. The online previewer uses `fetch`, so it needs HTTP(S) (not `file://`) for preview. The lecture slides themselves can still be opened locally.

## Adding or changing materials

Edit lecture files in the corresponding `Lecture_NN/` folder. If you add a **new student resource**, also update the matching entry in `assets/data.js`, so it appears in the homepage and previewer. Rebuild the related resources ZIP under `downloads/` if needed.

## Student controls

- `→` / `←` — next / previous slide
- `G` — table of contents
- `F` — fullscreen
- `B` — blackout
- `Print` — browser print or save slides to PDF
- `← Library` — return to the course homepage
- `Lxx` — previous or next lecture (when available)

No website framework, build system, database, or hosting charge is required for normal GitHub Pages use.
