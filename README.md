# adt7310_emulator
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)

# ADT7310について
[Analog Devices](https://www.analog.com/jp/index.html)社が開発販売する16bit温度センサ。

型番の7310はSPIによる通信をサポートし，7410はI2Cによる通信をサポートしている。

基本仕様は[Data Sheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ADT7310.pdf)参照。

# ADT7310エミュレータ
[Arduino](https://www.arduino.cc/)とAnalog Devices社のADT7310を使用した温度測定デバイスを想定する。

Linux上の環境でUNIX Socket Domainを経由してデータの送受信を行うエミュレータを実装した。

これはSPIのMasterから送られるコマンドビット列（あるいはコマンドバイト列）によってSPIスレーブのステータスレジスタの状態を書き換える。
1秒毎に温度の測定を行い，ソケット通信によって送出している。

# 使い方
- 環境
    + Ubuntu Linux 16.04 LTS
    + GCC 4.2.1

- 使い方
Makeによってコンパイルされる。

コンパイルによって生成されるバイナリは2種類。
    + s_e4adt7310
    + e4adt7310

s_e4adt7310（Simple_e4adt7310）は入力されるコマンドバイト列を2種類に絞っており，固定の温度情報を出力する。

e4adt7310は入力されるコマンドバイトを解釈し，適切な振る舞いをする。

- 実行
chmodなどによって実行権限を与える。

+ 引数
引数にLinux上でSPI通信を模倣するためのファイルディスクリプタが必要となる。

デフォルトで”/tmp/spi”が使用され，引数によって自由な位置に設定できる。

例：
```
./e4adt7310 ~/spi00
```

- debug
adt7310.hでは
```
#define PRINT_SOCK_COMM
```
が定義されている。

必要に合わせて利用する。

# 未実装のISSUE
- 温度生成に関わる関数（現在は固定値がセットされる。0x0DC6(16bit解像度で27.547℃)）
- Configurationコマンドを指定した際のの挙動

# 実装に際して
テスト環境に使用したADT7310用ライブラリは[scogswell/ArduinoADT7310](https://github.com/scogswell/ArduinoADT7310)です。


# 参考文献
[wsnakのブログ](http://www.wsnak.com/wsnakblog/?p=249)

[秋月電子通商](http://akizukidenshi.com/catalog/g/gM-06708/)

[Analog Devices](https://www.analog.com/jp/index.html)

[scogswell/ArduinoADT7310](https://github.com/scogswell/ArduinoADT7310)

