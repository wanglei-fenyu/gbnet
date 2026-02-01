import sys
from pathlib import Path
import chardet

TEXT_EXTS = {
    ".h", ".hpp", ".c", ".cpp", ".cc"
}

def convert_file(path: Path):
    try:
        raw = path.read_bytes()
        detect = chardet.detect(raw)
        enc = detect.get("encoding")

        if not enc:
            print(f"[skip] unknown encoding: {path}")
            return

        text = raw.decode(enc, errors="ignore")
        path.write_text(text, encoding="utf-8", newline="")

        print(f"[ok] {path} ({enc} -> utf-8)")
    except Exception as e:
        print(f"[err] {path}: {e}")

def convert_dir(root: Path):
    for path in root.rglob("*"):   # ⭐ 递归所有子文件夹
        if path.is_file() and path.suffix.lower() in TEXT_EXTS:
            convert_file(path)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: python convert_to_utf8.py <directory>")
        sys.exit(1)

    convert_dir(Path(sys.argv[1]))
