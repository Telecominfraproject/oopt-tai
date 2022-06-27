import unittest
import subprocess as sp
import threading
import os
import time
import taish
import asyncio

TAI_TEST_MODULE_LOCATION = os.environ.get("TAI_TEST_MODULE_LOCATION", "")
if not TAI_TEST_MODULE_LOCATION:
    TAI_TEST_MODULE_LOCATION = "0"

TAI_TEST_TAISH_SERVER_ADDRESS = os.environ.get("TAI_TEST_TAISH_SERVER_ADDRESS", "")
if not TAI_TEST_TAISH_SERVER_ADDRESS:
    TAI_TEST_TAISH_SERVER_ADDRESS = taish.DEFAULT_SERVER_ADDRESS

TAI_TEST_TAISH_SERVER_PORT = os.environ.get("TAI_TEST_TAISH_SERVER_PORT", "")
if not TAI_TEST_TAISH_SERVER_PORT:
    TAI_TEST_TAISH_SERVER_PORT = taish.DEFAULT_SERVER_PORT

TAI_TEST_NO_LOCAL_TAISH_SERVER = (
    True if os.environ.get("TAI_TEST_NO_LOCAL_TAISH_SERVER", "") else False
)


def output_reader(proc):
    for line in iter(proc.stdout.readline, b""):
        print("taish-server: {}".format(line.decode("utf-8")), end="")


class TestTAI(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        proc = sp.Popen("taish_server", stderr=sp.STDOUT, stdout=sp.PIPE)
        self.d = threading.Thread(target=output_reader, args=(proc,))
        self.d.start()
        self.proc = proc
        time.sleep(5)  # wait for the server to be ready

    def tearDown(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        self.proc.terminate()
        self.proc.wait(timeout=1)
        self.d.join()
        self.proc.stdout.close()

    async def test_list(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.list()
        self.assertNotEqual(m, None)
        self.assertTrue(TAI_TEST_MODULE_LOCATION in m)
        module = m[TAI_TEST_MODULE_LOCATION]
        print("module oid: 0x{:x}".format(module.oid))

        await cli.close()

    def test_taish_list(self):
        output = sp.run(
            [
                "taish",
                "--port",
                TAI_TEST_TAISH_SERVER_PORT,
                "--addr",
                TAI_TEST_TAISH_SERVER_ADDRESS,
                "-c",
                "list",
            ],
            capture_output=True,
        )
        self.assertEqual(output.returncode, 0)
        self.assertNotEqual(output.stdout.decode(), "")

    async def test_taish_remove(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.list()
        self.assertNotEqual(m, None)
        self.assertTrue(TAI_TEST_MODULE_LOCATION in m)
        module = m[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.netifs), 1)
        netif = module.netifs[0]
        oid = netif.oid

        cmd = f"remove 0x{oid:x}"

        output = sp.run(
            [
                "taish",
                "--port",
                TAI_TEST_TAISH_SERVER_PORT,
                "--addr",
                TAI_TEST_TAISH_SERVER_ADDRESS,
                "-c",
                cmd,
            ],
            capture_output=True,
        )

        self.assertEqual(output.returncode, 0)
        self.assertEqual(output.stderr.decode(), "")
        self.assertEqual(output.stdout.decode(), "")

        m = await cli.list()
        module = m[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.netifs), 0)

        await cli.close()

    async def test_get_module(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        self.assertNotEqual(m, None)
        self.assertEqual(m.location, TAI_TEST_MODULE_LOCATION)
        print("module oid: 0x{:x}".format(m.oid))
        await cli.close()

    async def test_get_netif(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        self.assertNotEqual(m, None)
        netif = m.get_netif()
        self.assertNotEqual(netif, None)
        self.assertEqual(netif.index, 0)
        print("netif oid: 0x{:x}".format(netif.oid))
        await cli.close()

    async def test_get_hostif(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        self.assertNotEqual(m, None)
        hostif = m.get_hostif()
        self.assertNotEqual(hostif, None)
        self.assertEqual(hostif.index, 0)
        print("hostif oid: 0x{:x}".format(hostif.oid))
        await cli.close()

    async def test_module(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        for metadata in await m.list_attribute_metadata():
            try:
                v = await m.get(metadata)
            except taish.TAIException as e:
                v = e.msg
            print("{}: {}".format(metadata.short_name, v))

        await cli.close()

    async def test_netif(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        netif = m.get_netif()
        for metadata in await netif.list_attribute_metadata():
            try:
                v = await netif.get(metadata)
            except taish.TAIException as e:
                v = e.msg
            print("{}: {}".format(metadata.short_name, v))

        await cli.close()

    async def test_netif_set_output_power(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        netif = m.get_netif()
        await netif.set("output-power", "-4")
        self.assertEqual(round(float(await netif.get("output-power"))), -4)
        await netif.set("output-power", "-5")
        self.assertEqual(round(float(await netif.get("output-power"))), -5)

        await cli.close()

    async def test_netif_set_modulation_format(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        netif = m.get_netif()
        await netif.set("modulation-format", "dp-64-qam")
        self.assertEqual(await netif.get("modulation-format"), "dp-64-qam")
        await netif.set("modulation-format", "dp-qpsk")
        self.assertEqual(await netif.get("modulation-format"), "dp-qpsk")

        await cli.close()

    async def test_hostif(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        hostif = m.get_hostif()
        for metadata in await hostif.list_attribute_metadata():
            try:
                v = await hostif.get(metadata)
            except taish.TAIException as e:
                v = e.msg
            print("{}: {}".format(metadata.short_name, v))

        await cli.close()

    async def test_hostif_set_fec(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        hostif = m.get_hostif()
        await hostif.set("fec-type", "rs")
        self.assertEqual(await hostif.get("fec-type"), "rs")
        await hostif.set("fec-type", "fc")
        self.assertEqual(await hostif.get("fec-type"), "fc")
        await hostif.set("fec-type", "none")
        self.assertEqual(await hostif.get("fec-type"), "none")

        await cli.close()

    async def test_hostif_set_loopback(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        hostif = m.get_hostif()
        await hostif.set("loopback-type", "shallow")
        self.assertEqual(await hostif.get("loopback-type"), "shallow")
        await hostif.set("loopback-type", "deep")
        self.assertEqual(await hostif.get("loopback-type"), "deep")
        await hostif.set("loopback-type", "none")
        self.assertEqual(await hostif.get("loopback-type"), "none")

        await cli.close()

    async def test_remove(self):

        output = sp.run(
            [
                "taish",
                "--port",
                TAI_TEST_TAISH_SERVER_PORT,
                "--addr",
                TAI_TEST_TAISH_SERVER_ADDRESS,
                "-c",
                "list",
            ],
        )

        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        l = await cli.list()
        self.assertNotEqual(l, None)
        self.assertTrue(TAI_TEST_MODULE_LOCATION in l)
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertNotEqual(module.oid, 0)
        self.assertEqual(len(module.netifs), 1)
        self.assertEqual(len(module.hostifs), 2)

        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        netif = m.get_netif()
        await cli.remove(netif.oid)
        l = await cli.list()
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.netifs), 0)

        hostif = m.get_hostif()
        await cli.remove(hostif.oid)
        l = await cli.list()
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.hostifs), 1)

        hostif = m.get_hostif(1)
        await cli.remove(hostif.oid)
        l = await cli.list()
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.hostifs), 0)

        await cli.remove(module.oid)
        l = await cli.list()
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(module.oid, 0)

        await cli.close()

    async def test_get_hostif(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        with self.assertRaises(taish.TAIException):
            m.get_hostif(10)

        hostif = m.get_hostif(0)
        self.assertNotEqual(hostif, None)
        await cli.remove(hostif.oid)

        hostif = m.get_hostif(1)
        self.assertNotEqual(hostif, None)

        # need to get module again for refresh
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        with self.assertRaises(taish.TAIException):
            m.get_hostif(0)

        hostif = m.get_hostif(1)
        self.assertNotEqual(hostif, None)

        await cli.close()

    async def test_create(self):
        await self.test_remove()
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        with self.assertRaises(Exception):
            await cli.get_module(TAI_TEST_MODULE_LOCATION)
        await cli.create("module", [("location", TAI_TEST_MODULE_LOCATION)])
        l = await cli.list()
        self.assertNotEqual(l, None)
        self.assertTrue(TAI_TEST_MODULE_LOCATION in l)
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertNotEqual(module.oid, 0)
        self.assertEqual(len(module.netifs), 0)
        self.assertEqual(len(module.hostifs), 0)

        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        self.assertEqual(int(await m.get("num-network-interfaces")), 1)
        self.assertEqual(int(await m.get("num-host-interfaces")), 2)

        await cli.create("netif", [("index", 0)], m.oid)
        l = await cli.list()
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.netifs), 1)

        hostif = await cli.create("hostif", [("index", 0)], m.oid)
        l = await cli.list()
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.hostifs), 1)

        await cli.create("hostif", [("index", 1)], m.oid)
        l = await cli.list()
        module = l[TAI_TEST_MODULE_LOCATION]
        self.assertEqual(len(module.hostifs), 2)

        await cli.close()

    async def test_module_create_hostif(self):
        await self.test_remove()
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.create_module(TAI_TEST_MODULE_LOCATION)
        hostif = await m.create_hostif(index=0)
        self.assertEqual(hostif.obj.module_oid, m.oid)

    async def test_get_set_multiple(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        hostif = m.get_hostif()
        v = await hostif.get_multiple(["loopback-type", "fec-type"])
        self.assertEqual(len(v), 2)
        self.assertEqual(v[0], "none")
        self.assertEqual(v[1], "none")

        await hostif.set_multiple([("loopback-type", "shallow"), ("fec-type", "rs")])

        v = await hostif.get_multiple(["loopback-type", "fec-type"])
        self.assertEqual(len(v), 2)
        self.assertEqual(v[0], "shallow")
        self.assertEqual(v[1], "rs")


class TestTAIWithConfig(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        proc = sp.Popen(
            ["taish_server", "-f", "config.json"], stderr=sp.STDOUT, stdout=sp.PIPE
        )
        self.d = threading.Thread(target=output_reader, args=(proc,))
        self.d.start()
        self.proc = proc
        time.sleep(5)  # wait for the server to be ready

    async def test_set_admin_status_attribute_module(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        self.assertNotEqual(m, None)
        print("module oid: 0x{:x}".format(m.oid))

        self.assertEqual(await m.get("admin-status"), "down")

        await m.set("admin-status", "up")

        self.assertEqual(await m.get("admin-status"), "up")
        await cli.close()

    async def test_set_custom_attribute_module(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        self.assertNotEqual(m, None)
        print("module oid: 0x{:x}".format(m.oid))

        self.assertEqual(await m.get("custom"), "true")

        await m.set("custom", "false")

        self.assertEqual(await m.get("custom"), "false")
        await cli.close()

    async def test_set_custom_list_attribute_module(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)
        self.assertNotEqual(m, None)
        print("module oid: 0x{:x}".format(m.oid))

        await m.set("custom-list", "1,2,3,4")

        self.assertEqual(await m.get("custom-list"), "1,2,3,4")

        await m.set("custom-list", "")
        self.assertEqual(await m.get("custom-list"), "")

        await cli.close()

    async def test_set_custom_list_attribute_module_taish(self):

        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.get_module(TAI_TEST_MODULE_LOCATION)

        output = sp.run(
            [
                "taish",
                "--port",
                TAI_TEST_TAISH_SERVER_PORT,
                "--addr",
                TAI_TEST_TAISH_SERVER_ADDRESS,
                "-c",
                f"module {TAI_TEST_MODULE_LOCATION}; set custom-list 1,2,3,4",
            ],
            capture_output=True,
        )

        self.assertEqual(output.returncode, 0)
        self.assertEqual(await m.get("custom-list"), "1,2,3,4")

        output = sp.run(
            [
                "taish",
                "--port",
                TAI_TEST_TAISH_SERVER_PORT,
                "--addr",
                TAI_TEST_TAISH_SERVER_ADDRESS,
                "-c",
                f"module {TAI_TEST_MODULE_LOCATION}; set custom-list",
            ],
            capture_output=True,
        )
        self.assertEqual(output.returncode, 0)
        self.assertEqual(await m.get("custom-list"), "")

        await cli.close()

    def tearDown(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        self.proc.terminate()
        self.proc.wait(timeout=1)
        self.d.join()
        self.proc.stdout.close()


class TestTAIWithoutObjectCreation(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        proc = sp.Popen(["taish_server", "-n"], stderr=sp.STDOUT, stdout=sp.PIPE)
        self.d = threading.Thread(target=output_reader, args=(proc,))
        self.d.start()
        self.proc = proc
        time.sleep(5)  # wait for the server to be ready

    async def test_list(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.list()
        self.assertNotEqual(m, None)
        self.assertTrue(TAI_TEST_MODULE_LOCATION in m)
        module = m[TAI_TEST_MODULE_LOCATION]
        print("module oid: 0x{:x}".format(module.oid))

        await cli.close()

    def test_taish_list(self):
        output = sp.run(
            [
                "taish",
                "--port",
                TAI_TEST_TAISH_SERVER_PORT,
                "--addr",
                TAI_TEST_TAISH_SERVER_ADDRESS,
                "-c",
                "list",
            ],
            capture_output=True,
        )
        self.assertEqual(output.returncode, 0)
        self.assertNotEqual(output.stdout.decode(), "")
        self.assertEqual(output.stderr.decode(), "")

    def tearDown(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        self.proc.terminate()
        self.proc.wait(timeout=1)
        self.d.join()
        self.proc.stdout.close()


if __name__ == "__main__":
    unittest.main()
