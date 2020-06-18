FROM docker:19.03.2
LABEL "repository"="https://github.com/elgohr/Publish-Docker-Github-Action"
LABEL "maintainer"="Lars Gohr"

RUN apk update \
  && apk upgrade \
  && apk add --no-cache git

ADD entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
