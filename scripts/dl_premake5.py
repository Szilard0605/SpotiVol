from pathlib import Path
import urllib.request 
import zipfile
import os

print("Looking for premake5...")

premake_exe = Path('../premake5/premake5.exe')
if premake_exe.is_file():
    print("premake5.exe is present.")
else:
    print("Couldn't find premake5.exe, downloading it...")
    dl_dir = Path("../premake5/")

    if not dl_dir.exists():
        os.makedirs(dl_dir)

    url = "https://github.com/premake/premake-core/releases/download/v5.0.0-beta8/premake-5.0.0-beta8-windows.zip"
    zip_path = str(dl_dir) + "premake-5.0.0-beta8-windows.zip"
    urllib.request.urlretrieve(url, zip_path)
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall("../premake5/")

    os.remove(zip_path)