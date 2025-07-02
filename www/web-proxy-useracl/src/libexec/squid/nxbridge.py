#!/usr/bin/env python3
import requests
import sys
import re
from urllib.parse import unquote
import syslog

NXHOST="localhost"
NXPORT=9080

while True:
    args = [unquote(x) for x in sys.stdin.readline().strip().split(" ")]
    [ domain ] = re.findall("(?:https?://)?([^/]+).*",args[0])
    r = requests.get(f'http://{NXHOST}:{NXPORT}/dnscat.jsp?domain={domain}')
    dnscat = r.text.strip()
    if dnscat in args:
        sys.stdout.writelines('OK\n')
    else:
        sys.stdout.writelines('ERR\n')
    sys.stdout.flush()
sys.exit(0)
