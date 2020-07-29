FROM alpine:3.9

VOLUME /opt/certs

COPY publish /usr/bin

RUN apk add --no-cache bash

RUN chmod +x /usr/bin/publish

CMD ["./usr/bin/publish", "0", "0", "0", "400"]

USER nobody