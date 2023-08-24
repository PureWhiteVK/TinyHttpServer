# TinyHttpServer
tiny c++ http server implementation with asio

# Dependencies

- asio (without boost)
- spdlog
- json

# HTTP message format

```txt
HTTP-message   = start-line CRLF *( field-line CRLF ) CRLF [ message-body ]
start-line     = request-line 
               / status-line
request-line   = method SP request-target SP HTTP-version
method         = token
// here we made some simplication, we should use url decoder to get further infomation (for this part we can use antlr)
request-target = *<OCTET except ctl>
HTTP-version   = HTTP-name "/" DIGIT "." DIGIT
HTTP-name      = %s"HTTP"
status-line    = HTTP-version SP status-code SP [ reason-phrase ]
status-code    = 3DIGIT
reason-phrase  = 1*( HTAB / SP / VCHAR / obs-text )
VCHAR          = %x20-7E
obs-text       = %x80-FF
SP             = %x20
HTAB           = %x9
field-line     = field-name ":" OWS field-value OWS
OWS            = *( SP / HTAB )
field-name     = token
field-value    = *field-content
field-content  = field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
field-vchar    = VCHAR / obs-text
message-body   = *OCTET
token          = 1*tchar
tchar          = "!" / "#" / "$" / "%" / "&" / "'" / "*"
                / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
                / DIGIT / ALPHA
                ; any VCHAR, except delimiters
```

# TODO

- [x] Add HTTP/1.0 (just GET request) Demo from boost::asio example
- [ ] Add HTTP/1.1 implementation (GET + POST)
- [ ] Add HTTPS implementation with asio::ssl
- [ ] Add mysql connection (c++ connector) like TinyWebServer
- [ ] Add pressure test compared with [TinyWebServer](https://github.com/qinguoyi/TinyWebServer)