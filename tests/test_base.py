import unittest
import subprocess as sp
import threading
import os
import time
import taish

def output_reader(proc):
    for line in iter(proc.stdout.readline, b''):
        print('taish-server: {}'.format(line.decode('utf-8')), end='')

class TestTAI(unittest.TestCase):

    def setUp(self):
        proc = sp.Popen('taish_server', stderr=sp.STDOUT, stdout=sp.PIPE)
        self.d = threading.Thread(target=output_reader, args=(proc,))
        self.d.start()
        self.proc = proc
        time.sleep(5) # wait for the server to be ready

    def tearDown(self):
        print('tear down')
        self.proc.terminate()
        self.proc.wait(timeout=0.2)
        self.d.join()
        self.proc.stdout.close()

    def test_list(self):
        cli = taish.Client()
        m = cli.list()
        self.assertNotEqual(m, None)
        self.assertTrue('0' in m)
        module = m['0']
        print('module oid: 0x{:x}'.format(module.oid))

    def test_get_module(self):
        cli = taish.Client()
        m = cli.get_module('0')
        self.assertNotEqual(m, None)
        print('module oid: 0x{:x}'.format(m.oid))

    def test_get_netif(self):
        cli = taish.Client()
        m = cli.get_module('0')
        self.assertNotEqual(m, None)
        netif = m.get_netif()
        self.assertNotEqual(netif, None)
        print('netif oid: 0x{:x}'.format(netif.oid))

    def test_get_hostif(self):
        cli = taish.Client()
        m = cli.get_module('0')
        self.assertNotEqual(m, None)
        hostif = m.get_hostif()
        self.assertNotEqual(hostif, None)
        print('hostif oid: 0x{:x}'.format(hostif.oid))

    def test_module(self):
        cli = taish.Client()
        m = cli.get_module('0')
        for metadata in m.list_attribute_metadata():
            try:
                v = m.get(metadata)
            except taish.TAIException as e:
                v = e.msg
            print('{}: {}'.format(metadata.short_name, v))

    def test_netif(self):
        cli = taish.Client()
        m = cli.get_module('0')
        netif = m.get_netif()
        for metadata in netif.list_attribute_metadata():
            try:
                v = netif.get(metadata)
            except taish.TAIException as e:
                v = e.msg
            print('{}: {}'.format(metadata.short_name, v))

    def test_netif_set_output_power(self):
        cli = taish.Client()
        m = cli.get_module('0')
        netif = m.get_netif()
        netif.set('output-power', '-4')
        self.assertEqual(round(float(netif.get('output-power'))), -4)
        netif.set('output-power', '-5')
        self.assertEqual(round(float(netif.get('output-power'))), -5)

    def test_netif_set_modulation_format(self):
        cli = taish.Client()
        m = cli.get_module('0')
        netif = m.get_netif()
        netif.set('modulation-format', 'dp-64-qam')
        self.assertEqual(netif.get('modulation-format'), '"dp-64-qam"')
        netif.set('modulation-format', 'dp-qpsk')
        self.assertEqual(netif.get('modulation-format'), '"dp-qpsk"')

    def test_hostif(self):
        cli = taish.Client()
        m = cli.get_module('0')
        hostif = m.get_hostif()
        for metadata in hostif.list_attribute_metadata():
            try:
                v = hostif.get(metadata)
            except taish.TAIException as e:
                v = e.msg
            print('{}: {}'.format(metadata.short_name, v))

    def test_hostif_set_fec(self):
        cli = taish.Client()
        m = cli.get_module('0')
        hostif = m.get_hostif()
        hostif.set('fec-type', 'rs')
        self.assertEqual(hostif.get('fec-type'), '"rs"')
        hostif.set('fec-type', 'fc')
        self.assertEqual(hostif.get('fec-type'), '"fc"')
        hostif.set('fec-type', 'none')
        self.assertEqual(hostif.get('fec-type'), '"none"')

    def test_hostif_set_loopback(self):
        cli = taish.Client()
        m = cli.get_module('0')
        hostif = m.get_hostif()
        hostif.set('loopback-type', 'shallow')
        self.assertEqual(hostif.get('loopback-type'), '"shallow"')
        hostif.set('loopback-type', 'deep')
        self.assertEqual(hostif.get('loopback-type'), '"deep"')
        hostif.set('loopback-type', 'none')
        self.assertEqual(hostif.get('loopback-type'), '"none"')


if __name__ == '__main__':
    unittest.main()
