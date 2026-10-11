#!/usr/bin/env python3
"""Package a successfully built/tested beta, preserving third-party notices."""
import argparse, hashlib, os, pathlib, struct, subprocess, zipfile
def validate_binary(raw, platform):
    """Accept x64 PE DLLs and matching thin Mach-O dylibs or loadable bundles."""
    if platform=="windows-x64":
        if len(raw)<64 or raw[:2]!=b"MZ":
            raise SystemExit("Expected a Windows PE DLL")
        offset=struct.unpack_from("<I",raw,0x3c)[0]
        if offset+24>len(raw) or raw[offset:offset+4]!=b"PE\0\0" or struct.unpack_from("<H",raw,offset+4)[0]!=0x8664 or not struct.unpack_from("<H",raw,offset+22)[0]&0x2000:
            raise SystemExit("Expected an x64 DLL")
    else:
        expected=0x100000c if platform=="macos-arm64" else 0x1000007
        if len(raw)<32 or raw[:4]!=b"\xcf\xfa\xed\xfe" or struct.unpack_from("<I",raw,4)[0]!=expected or struct.unpack_from("<I",raw,12)[0] not in (6,8):
            raise SystemExit("Expected a matching thin Mach-O dylib or bundle")

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--platform", choices=["macos-intel", "macos-arm64", "windows-x64"], required=True)
    p.add_argument("--binary", type=pathlib.Path, required=True)
    a=p.parse_args()
    root=pathlib.Path(__file__).resolve().parents[1]
    version="0.3.2"
    validate_binary(a.binary.read_bytes(), a.platform)
    if a.platform.startswith("macos"):
        subprocess.run(["codesign", "--force", "--sign", "-", str(a.binary)], check=True)
    binary=a.binary.read_bytes()
    filename="reaper_roto.dll" if a.platform=="windows-x64" else "reaper_roto.dylib"
    folder=f"roto-reaper-v{version}-{a.platform}"
    output=root/"dist"/(folder+".zip")
    output.parent.mkdir(exist_ok=True)
    status="Experimental build. Physical hardware validation required for this asset."
    install=f"""Roto-Control for REAPER v{version} — {a.platform} experimental beta
    
    {status}
    Use matching native REAPER architecture: Windows x64, Intel Mac x86_64,
    or Apple Silicon Mac ARM64 as labeled. These are separate builds.
    
    1. In REAPER choose Options > Show REAPER resource path in explorer/finder.
    2. Quit REAPER normally.
    3. Copy {filename} into the UserPlugins folder there, replacing an older build.
    4. Reopen REAPER. Disable ordinary Roto-Control MIDI input/output under
       Preferences > Audio > MIDI Devices so the control surface can own the ports.
    5. Under Preferences > Control/OSC/web add Roto-Control REAPER once if absent.
       There should be exactly one matching MIDI input and output.
    6. Use MIX all tracks. See README.md and SEND-TEST.md for hardware checks.
    
    Existing learned mappings and saved control-surface configuration are retained.
    Known limitation: touch OFF may arrive during a continuous physical hold,
    reproduced outside REAPER. Touch automation holds remain unreliable.
    Send automation and explicit plugin automation touch reporting are not validated.
    No firmware modification or Bitwig installation is required.
    
    To remove: quit REAPER, remove {filename} from UserPlugins, then remove
    its control-surface entry in REAPER. To roll back, replace it with the old binary.
    Report feedback at https://github.com/broisme-bit/roto-reaper/issues
    Include OS, REAPER architecture/version, firmware, test steps and roto-reaper.log.
    Do not post private project information in public logs.
    """
    with zipfile.ZipFile(output,"w",zipfile.ZIP_DEFLATED) as z:
        z.writestr(folder+"/"+filename,binary)
        z.writestr(folder+"/INSTALL.txt",install)
        z.writestr(folder+"/SHA256SUMS.txt",hashlib.sha256(binary).hexdigest()+"  "+filename+"\n")
        z.writestr(folder+"/BUILD-INFO.txt",f"Platform: {a.platform}\nVersion: {version}\nCommit: {os.environ.get('GITHUB_SHA','local build')}\nStatus: {status}\n")
        for name in ["LICENSE","THIRD-PARTY-NOTICES.txt","README.md","SEND-TEST.md","RECONNECT-TEST.md","SINGLE-TRACK-TEST.md"]:
            z.write(root/name,folder+"/"+name)
    with zipfile.ZipFile(output) as z:
        assert z.testzip() is None
    print(output)

if __name__ == "__main__":
    main()
