ARG IMAGE="debian:12-slim"
FROM docker.io/library/$IMAGE

ARG IMAGE="debian:12-slim"
ARG ALT="-dev"
ARG TAG="20230802@cfc349e"

ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update
RUN apt-get install -y --no-install-recommends --no-install-suggests \
	ca-certificates wget p7zip-full \
	libicu-dev libexpat1-dev libsdl2-dev libpng-dev libpixman-1-dev \
	libfmt-dev libfreetype6-dev libharfbuzz-dev libmpg123-dev \
	libsndfile-dev libvorbis-dev libopusfile-dev libspeexdsp-dev \
	libdrm-dev libgbm-dev

WORKDIR /opt

RUN \
	IMG="$(echo $IMAGE | sed 's/:/./')"; \
	wget "https://github.com/monokotech/EasyRPG-Multiplayer-Native$ALT/releases/download/$TAG/Linux-Build-$IMG-$TAG.zip" && \
	7z x -o/ "Linux-Build-$IMG-$TAG.zip"

EXPOSE 6500/tcp
VOLUME /opt/config

ENTRYPOINT ["/usr/bin/easyrpg-player"]
CMD ["--server", "--bind-address", "0.0.0.0:6500", "--config-path", "/opt/config/Player"]
