FROM alpine:3.11 AS base
WORKDIR /app

# Add ncurses package to base
RUN apk update
RUN apk add ncurses-dev

# Create build from base image
FROM base AS build

# Add make, gcc, and libc-dev packages
RUN apk add make gcc libc-dev

# Copy everything to workdir, and remove built fireplace executable (-f will not fail if doesn't exist)
COPY . .
RUN rm -f fireplace
RUN make

# Create run image from base
FROM base AS final

# Set term colors
ENV TERM=xterm-256color

# Copy fireplace executable from build image to final
WORKDIR /app
COPY --from=build /app/fireplace .
ENTRYPOINT [ "/app/fireplace" ]
