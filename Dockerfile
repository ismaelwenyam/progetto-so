FROM ubuntu:latest
RUN apt update && apt install -y build-essential make
WORKDIR /app
COPY . .
RUN make
CMD ["./direttore"]
