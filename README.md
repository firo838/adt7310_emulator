# adt7310_emulator
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

# ADT7310について
[Analog Devices](https://www.analog.com/jp/index.html)社が開発販売する16bit温度センサ。

型番の7310はSPIによる通信をサポートし，7410はI2Cによる通信をサポートしている。

基本仕様は[Data Sheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ADT7310.pdf)参照。

# ADT7310エミュレータ
[Arduino](https://www.arduino.cc/)とAnalog Devices社のADT7310を使用した温度測定デバイスを想定する。

Linux上の環境でUNIX Socket Domainを経由してデータの送受信を行うエミュレータを実装した。

これはSPIのMasterから送られるコマンドバイト列によってSPIスレーブのコンフィギュレーションレジスタの状態を書き換える。
Continuous modeのとき逐次temp_genによって温度を生成して，ソケット通信によって送出している。

# 使い方
## 環境
    * Ubuntu Linux 16.04 LTS
    * GCC 4.2.1

## 使い方
Makeによってコンパイルされる。

コンパイルによって生成されるバイナリはe4adt7310である。

e4adt7310は入力されるコマンドバイトを解釈し，適切な振る舞いをする。

## 実行

chmodなどによって実行権限を与える。

## 引数

引数にLinux上でSPI通信を模倣するためのファイルディスクリプタが必要となる。

デフォルトで”/tmp/var/spi”が使用され，引数によって自由な位置に設定できる。

例：
```
./e4adt7310 ~/spi00
```

## debugに関して

adt7310.hでは
```
#define PRINT_SOCK_COMM
#define PRINT_SOCK_COMM_DEBUG
#define DEBUG_PRINT
```
が定義されている。

必要に合わせて利用する。


# 参考文献
[wsnakのブログ](http://www.wsnak.com/wsnakblog/?p=249)

[秋月電子通商](http://akizukidenshi.com/catalog/g/gM-06708/)

[Analog Devices](https://www.analog.com/jp/index.html)

