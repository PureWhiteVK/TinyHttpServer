import asyncio
import tornado
import tornado.web
import pathlib
import ssl

ROOT = pathlib.Path(__file__).parents[1]

async def main():
    ssl_ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
    ssl_ctx.options &= ssl.OP_ALL
    ssl_ctx.load_cert_chain(ROOT / 'data' / 'CA' / 'cert.pem',
                            ROOT / 'data' / 'CA' / 'key.pem')
    application = tornado.web.Application([
        (r"/(.*)",tornado.web.StaticFileHandler,{'path': ROOT / 'static', 'default_filename': 'index.html'})
    ])
    application.listen(8713,address='0.0.0.0',ssl_options=ssl_ctx,idle_connection_timeout=10.0)
    await asyncio.Event().wait()

if __name__ == "__main__":
    asyncio.run(main())