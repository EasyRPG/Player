ARG IMAGE="debian:12-slim"
FROM docker.io/library/$IMAGE

ARG IMAGE="debian:12-slim"
ARG ALT="-dev"
ARG TAG="test"

ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update
RUN apt-get install -y --no-install-recommends --no-install-suggests \
	ca-certificates wget p7zip-full libicu-dev libexpat1-dev libfmt-dev

WORKDIR /opt

RUN \
	IMG="$(echo $IMAGE | sed 's/:/./')"; \
	URL="https://github.com/monokotech/EasyRPG-Multiplayer-Native$ALT"; \
	URL="$URL/releases/download/$TAG/Linux-Server-Build-$IMG-$TAG.zip"; \
	wget "$URL" && 7z x -o/ "Linux-Server-Build-$IMG-$TAG.zip"; \
	rm -rfv "Linux-Server-Build-$IMG-$TAG.zip"

EXPOSE 6500/tcp
VOLUME /opt/config

ENTRYPOINT ["/usr/bin/easyrpg-player-server"]
CMD ["--bind-address", "0.0.0.0:6500", "--config-path", "/opt/config/server.ini"]
