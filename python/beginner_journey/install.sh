#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

PYTHON_BIN="${PYTHON_BIN:-python3}"

echo "[1/5] Check Python"
if ! command -v "${PYTHON_BIN}" >/dev/null 2>&1; then
  echo "ERROR: ${PYTHON_BIN} not found."
  echo "Try: PYTHON_BIN=python3.10 ./install.sh"
  exit 1
fi

"${PYTHON_BIN}" - <<'PY'
import sys

print("Python executable:", sys.executable)
print("Python version:", sys.version)

if sys.version_info[:2] != (3, 10):
    raise SystemExit(
        "ERROR: Bajie SDK wheels in this demo require Python 3.10. "
        "Run with: PYTHON_BIN=python3.10 ./install.sh"
    )
PY

echo
echo "[2/5] Check pip"
"${PYTHON_BIN}" -m pip --version >/dev/null 2>&1 || {
  echo "ERROR: pip is not available for ${PYTHON_BIN}."
  echo "Install python3-pip first, then rerun this script."
  exit 1
}
"${PYTHON_BIN}" -m pip --version

echo
echo "[3/5] Check Jupyter"
if "${PYTHON_BIN}" -m notebook --version >/dev/null 2>&1; then
  echo "Jupyter Notebook already installed."
else
  echo "Jupyter Notebook not found. Installing notebook..."
  "${PYTHON_BIN}" -m pip install --user notebook
fi

echo
echo "[4/5] Verify Jupyter"
"${PYTHON_BIN}" -m notebook --version

echo
echo "[5/5] Validate notebooks"
"${PYTHON_BIN}" -m py_compile helpers.py generate_notebooks.py
"${PYTHON_BIN}" - <<'PY'
import json
from pathlib import Path

paths = sorted(Path(".").glob("*.ipynb"))
if not paths:
    raise SystemExit("ERROR: no .ipynb files found in beginner_journey.")

for path in paths:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        raise SystemExit(f"ERROR: {path}: invalid JSON: {exc}") from exc

    if data.get("nbformat") != 4:
        raise SystemExit(f"ERROR: {path}: nbformat={data.get('nbformat')!r}, expected 4")

    cells = data.get("cells")
    if not isinstance(cells, list) or not cells:
        raise SystemExit(f"ERROR: {path}: missing cells")

    for idx, cell in enumerate(cells):
        cell_type = cell.get("cell_type")
        if cell_type not in {"markdown", "code", "raw"}:
            raise SystemExit(f"ERROR: {path}: cell {idx} invalid cell_type={cell_type!r}")
        if "source" not in cell:
            raise SystemExit(f"ERROR: {path}: cell {idx} missing source")

    print(f"OK {path} ({len(cells)} cells)")
PY

echo
echo "Install check complete."
echo
echo "Open the course with:"
echo "  cd ${SCRIPT_DIR}"
echo "  ${PYTHON_BIN} -m jupyter notebook"
