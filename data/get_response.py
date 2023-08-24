import requests
import dataclasses


def save_raw(name: str, url: str):
    resp = requests.get(url, stream=True)
    print(resp.headers)
    print(f'Content-Encoding: {resp.headers["Content-Encoding"]}')
    print(f'save {name}_raw.bin')
    if 'Transfer-Encoding' in resp.headers and resp.headers['Transfer-Encoding'] == 'chunked':
        print(f'write chunked data')
        with open(f'{name}_raw.bin', 'wb') as f:
            for chunk in resp.raw.read_chunked(1024):
                f.write(chunk)
    else:
        with open(f'{name}_raw.bin', 'wb') as f:
            f.write(resp.raw.read())


def save_decoded(name: str, url: str):
    resp = requests.get(url, stream=True)
    print(f'Content-Encoding: {resp.headers["Content-Encoding"]}')
    print(f'save {name}_decoded.bin')
    with open(f'{name}_decoded.bin', 'wb') as f:
        for chunk in resp.iter_content(chunk_size=1024):
            f.write(chunk)


@dataclasses.dataclass
class TestRequestData:
    name: str
    url: str


urls = [
    TestRequestData('deflate', 'https://postman-echo.com/deflate'),
    TestRequestData('gzip', 'https://postman-echo.com/gzip'),
    TestRequestData('br', 'https://zhuanlan.zhihu.com/p/166359481')
]

for url in urls:
    save_decoded(url.name, url.url)
    save_raw(url.name, url.url)
