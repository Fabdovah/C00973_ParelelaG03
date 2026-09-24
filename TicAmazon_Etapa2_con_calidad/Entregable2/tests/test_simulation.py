import pathlib
import subprocess
import unittest

BASE = pathlib.Path(__file__).resolve().parents[1] / 'src' / 'Simulacion'

class SimulationSmokeTest(unittest.TestCase):
    def test_registration_query_and_clean_exit(self):
        executable = BASE / 'simulación'
        if not executable.exists():
            self.skipTest('Compile simulation first')
        result = subprocess.run([str(executable), 'almacen', '1', '2'], cwd=BASE,
                                input='1\n4\n', text=True, capture_output=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('SIMULACION TICAMAZON', result.stdout)
        self.assertIn('RESPUESTA', result.stdout)

if __name__ == '__main__':
    unittest.main(verbosity=2)
