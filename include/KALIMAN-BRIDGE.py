"""Puente local de solo recepción; requiere Python 3."""
from collections import deque
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
from threading import Thread, Lock
from xmlrpc.client import ServerProxy
from datetime import timedelta
import locale
import sys
from html import escape
import json
import socket
import struct
import time

def language():
    if sys.platform == 'win32':
        import ctypes
        lang = ctypes.windll.kernel32.GetUserDefaultUILanguage()
        return 'es' if (lang & 0x3ff) == 10 else 'en'
    return 'es' if (locale.getlocale()[0] or '').lower().startswith('es') else 'en'


LANG = language()


def tr(es, en):
    return es if LANG == 'es' else en


HOST = '127.0.0.1'
lock = Lock()
lines, cw = deque(maxlen=100), deque(maxlen=100)
status = {'cw': 'waiting', 'ft8': 0}


def decode(data):
    magic, schema, kind = struct.unpack_from('>III', data)
    if magic != 0xadbccbda or schema not in (2, 3) or kind not in (0, 2):
        return None
    pos = 12

    def text():
        nonlocal pos
        size, = struct.unpack_from('>I', data, pos)
        pos += 4
        if size > len(data) - pos:
            raise ValueError('Trama incompleta')
        result = data[pos:pos+size].decode('utf-8', 'replace')
        pos += size
        return result

    text()  # ID del emisor
    if kind == 0:
        return ''
    new, ms, snr, dt, hz = struct.unpack_from('>?IidI', data, pos)
    pos += 21
    mode, message = text(), text()
    low = pos < len(data) and data[pos]
    off_air = pos+1 < len(data) and data[pos+1]
    if not new or off_air or mode != '~' or ms >= 86400000:
        return ''
    return f'{timedelta(seconds=ms//1000)} UTC  {snr:+d} dB  {dt:+.1f} s  {hz} Hz  {message}' + (tr(' [dudoso]', ' [low confidence]') if low else '')


def listen(udp):
    while True:
        try:
            line = decode(udp.recvfrom(8192)[0])
        except (ValueError, struct.error):
            continue
        if line is not None:
            with lock:
                status['ft8'] = time.time()
                if line:
                    lines.append(line)


def read_cw():
    while True:
        try:
            with ServerProxy('http://127.0.0.1:7362') as proxy:
                ready = proxy.modem.get_name() == 'CW'
                chunk = proxy.rx.get_data().data if ready else b''
            with lock:
                status['cw'] = 'ready' if ready else 'mode'
                if chunk:
                    cw.append(chunk[-4096:].decode('utf-8', 'replace'))
        except Exception:
            with lock:
                status['cw'] = 'offline'
        time.sleep(1)


class Panel(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.headers.get('Host') != '127.0.0.1:8765':
            self.send_error(403)
            return
        if self.path == '/':
            body = Path(__file__).with_name('KALIMAN-DASHBOARD.html').read_bytes()
            mime = 'text/html; charset=utf-8'
        elif self.path == '/reception-tools':
            title = tr('Herramientas de recepción', 'Reception Tools')
            guide = tr('RECEPTION-TOOLS.md', 'RECEPTION-TOOLS.en.md')
            text = escape(Path(__file__).with_name(guide).read_text(encoding='utf-8'))
            body = (f'<!doctype html><html lang="{LANG}"><meta charset="utf-8">'
                    f'<meta name="viewport" content="width=device-width,initial-scale=1">'
                    f'<title>{title}</title><style>body{{max-width:75ch;margin:2rem auto;'
                    f'padding:0 1rem;font:1rem system-ui}}pre{{white-space:pre-wrap;font:inherit}}'
                    f'</style><a href="/">{tr("Volver al panel", "Back to dashboard")}</a>'
                    f'<pre>{text}</pre></html>').encode()
            mime = 'text/html; charset=utf-8'
        elif self.path == '/decode':
            with lock:
                body = json.dumps(dict(status, lang=LANG, lines=list(lines), morse=''.join(cw)[-8000:])).encode()
            mime = 'application/json'
        else:
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header('Content-Type', mime)
        self.send_header('Cache-Control', 'no-store')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *args):
        pass


if __name__ == '__main__':
    socket.setdefaulttimeout(3)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp, HTTPServer((HOST, 8765), Panel) as server:
        udp.bind((HOST, 2237))
        udp.settimeout(None)
        Thread(target=listen, args=(udp,), daemon=True).start()
        Thread(target=read_cw, daemon=True).start()
        print(tr('Abre http://127.0.0.1:8765 en Chrome o Edge. Ctrl+C para cerrar.',
                 'Open http://127.0.0.1:8765 in Chrome or Edge. Press Ctrl+C to exit.'), flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
