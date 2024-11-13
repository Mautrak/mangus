FROM ubuntu:22.04

# Bu kurulum sırasında bişey sormaya kalkmasın diye
ENV DEBIAN_FRONTEND=noninteractive

# Bunlari kurmamız lazım yoksa çalışmaz
RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    && rm -rf /var/lib/apt/lists/*

# Hadi bi tane klasör açalım herşeyi buraya atacağız
WORKDIR /app

# Bütün dosyaları buraya kopyalıyoruz
COPY . .

# Şimdi src klasörüne girip make yapıcaz, kolay bişey
RUN cd src && make

# Mangus'u area klasörüne taşıyoruz çünkü öyle çalışıyo bu sistem
RUN mv src/mangus area/

# Area klasörüne geçiyoruz mangus burdan başlıyor
WORKDIR /app/area

# Mangus'u çalıştırıyoruz, işte bu kadar
CMD ["./mangus"]
