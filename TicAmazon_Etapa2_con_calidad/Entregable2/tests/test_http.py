"""HTTP integration tests. Start an isolated plaintext product server (no localhost)."""
import concurrent.futures
import http.client
import json
import os
import pathlib
import shutil
import socket
import subprocess
import tempfile
import time
import unittest
import urllib.parse

SERVER = pathlib.Path(__file__).resolve().parents[1] / 'src' / 'ServidorProductos' / 'servidor'


def lan_host():
    """Use a non-loopback IPv4 address; override TIC_HOST in a lab network."""
    if os.environ.get('TIC_HOST'):
        return os.environ['TIC_HOST']
    for family, _, _, _, address in socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET):
        if family == socket.AF_INET and not address[0].startswith('127.'):
            return address[0]
    raise unittest.SkipTest('No non-loopback IPv4; export TIC_HOST with the assigned laboratory IP')


class HTTPTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not SERVER.is_file():
            raise unittest.SkipTest('Compile first: make -C src/ServidorProductos')
        cls.host = lan_host()
        cls.tmp = tempfile.TemporaryDirectory(prefix='ticamazon_http_')
        cls.cwd = pathlib.Path(cls.tmp.name)
        (cls.cwd / 'almacen').mkdir()
        sock = socket.socket()
        sock.bind((cls.host, 0))
        cls.port = sock.getsockname()[1]
        sock.close()
        cls.proc = subprocess.Popen([str(SERVER)], cwd=cls.cwd, stdin=subprocess.PIPE,
                                    stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        cls.proc.stdin.write(f'1\n{cls.port}\n2\n')
        cls.proc.stdin.flush()
        for _ in range(80):
            try:
                cls.get('/categories')
                break
            except (OSError, http.client.HTTPException):
                if cls.proc.poll() is not None:
                    raise RuntimeError('Server exited during startup')
                time.sleep(0.1)
        else:
            cls.tearDownClass()
            raise RuntimeError('Server startup timed out')

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, 'proc'):
            cls.proc.terminate()
            try:
                cls.proc.communicate(timeout=3)
            except subprocess.TimeoutExpired:
                cls.proc.kill()
                cls.proc.communicate()
        if hasattr(cls, 'tmp'):
            cls.tmp.cleanup()

    @classmethod
    def request(cls, method, path, data=None, headers=None):
        conn = http.client.HTTPConnection(cls.host, cls.port, timeout=5)
        try:
            conn.request(method, path, body=data, headers=headers or {})
            response = conn.getresponse()
            return response.status, dict(response.getheaders()), response.read().decode()
        finally:
            conn.close()

    @classmethod
    def get(cls, path, json_accept=True):
        headers = {'Accept': 'application/json'} if json_accept else {}
        return cls.request('GET', path, headers=headers)

    def test_01_categories_json(self):
        code, headers, body = self.get('/categories')
        self.assertEqual(code, 200)
        self.assertIn('application/json', headers['Content-Type'])
        self.assertIn('Frutas', [i['nombre'] for i in json.loads(body)['categorias']])

    def test_02_categories_html(self):
        code, _, body = self.get('/categories', json_accept=False)
        self.assertEqual(code, 200)
        self.assertIn('<html', body)

    def test_03_products(self):
        code, _, body = self.get('/products?category=Frutas')
        self.assertEqual(code, 200)
        self.assertTrue(json.loads(body)['productos'])
        self.assertTrue(all(x['categoria'] == 'Frutas' for x in json.loads(body)['productos']))

    def test_04_missing_filter(self):
        code, _, body = self.get('/products')
        self.assertEqual(code, 400)
        self.assertIn('INVALID_PARAMETERS', body)

    def test_05_unknown_endpoint(self):
        code, _, _ = self.get('/ruta_inexistente')
        self.assertEqual(code, 404)

    def test_06_buy_validation(self):
        for payload in ({}, {'codigo': 1, 'warehouse': 1, 'quantity': 0},
                        {'codigo': 1, 'warehouse': 1, 'quantity': 'invalid'}):
            encoded = urllib.parse.urlencode(payload)
            code, _, _ = self.request('POST', '/buy', encoded, {'Content-Type': 'application/x-www-form-urlencoded'})
            self.assertEqual(code, 400)

    def test_07_buy_stock_and_persistence(self):
        code, _, body = self.get('/products?category=Frutas')
        product = next(p for p in json.loads(body)['productos'] if p['nombre'] == 'Manzana')
        payload = {'codigo': product['id'], 'warehouse': product['warehouse'], 'quantity': 1}
        code, _, body = self.request('POST', '/buy', urllib.parse.urlencode(payload),
                                     {'Content-Type': 'application/x-www-form-urlencoded'})
        self.assertEqual(code, 200)
        self.assertEqual(json.loads(body)['remaining'], product['stock'] - 1)
        code, _, body = self.get('/products?category=Frutas')
        current = next(p for p in json.loads(body)['productos'] if p['nombre'] == 'Manzana')
        self.assertEqual(current['stock'], product['stock'] - 1)
        code, _, body = self.request('POST', '/buy', urllib.parse.urlencode({**payload, 'quantity': 999999}),
                                     {'Content-Type': 'application/x-www-form-urlencoded'})
        self.assertEqual(code, 409)
        self.assertIn('INSUFFICIENT_STOCK', body)

    def test_08_parallel_buy_no_oversell(self):
        products = json.loads(self.get('/products?category=Frutas')[2])['productos']
        product = next(p for p in products if p['nombre'] == 'Naranja')
        amount = product['stock']
        payload = urllib.parse.urlencode({'codigo': product['id'], 'warehouse': product['warehouse'], 'quantity': amount})
        def buy(_):
            return self.request('POST', '/buy', payload, {'Content-Type': 'application/x-www-form-urlencoded'})[0]
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
            statuses = list(executor.map(buy, range(2)))
        self.assertEqual(sorted(statuses), [200, 409])


if __name__ == '__main__':
    unittest.main(verbosity=2)
