# TSPInstanceMaker

## ENVIRONMENT
opencv/C++ Windows x64 visual studio2010

## HOW TO USE
### BUILD
visual studio 2017で実行してください．

環境によっては、ソリューションファイルをダブルクリックしても実行できない場合があります。
その場合は、visual studioを立ち上げて「ファイル」→「ファイルを開く」から実行してください。

「コンピューターにMSVCP120.dllがないため、プログラムを開始できません。」と出て実行できない場合、
以下のサイトより、Visual Studio 2013 の再頒布可能パッケージをインストールしてください。

https://www.microsoft.com/ja-JP/download/details.aspx?id=40784

「WindowsSDKバージョン8.1が見つかりませんでした。」と出て実行できない場合、
以下のサイトを参考にSDKをインストールしてください。

http://dxlib.o.oo7.jp/cgi/patiobbs/patio.cgi?mode=view&no=4107

### PROGRAM
main()関数のoption specifyの部分を変更して実行します．
実行時に必要な画像は`TSPInstanceMaker/img/`に入れてください．

### OUTPUT
`TSPInstanceMaker/tsp`に結果のtspファイルが出力されます．
`DEBUG_OUTPUT=true`のときは`TSPInstanceMaker/tmp`にボロノイ領域を可視化したものがデバッグ出力されます．

### UTILITIES
`TSPInstanceMaker/util`にお役たちのpythonコードがあります．
以下に説明を記します．なお，$で始まる行はコマンドライン上での使用方法を示しており，斜体は引数を意味します．

---

#### point_delete.py
$python point_delete.py _tsp-file-path_

_tsp-file-path_ で指定したTSPインスタンスから，対話形式で都市を削除します．
**指定した座標に最も近い都市を削除していきます．プログラムを終了するときはendと入力してください．**

**python3でないと，`input()`でエラーがでます．**

---

#### point_plot.py
$python point_plot.py _tsp-file-path_

_tsp-file-path_ で指定したTSPインスタンスを画像に出力します．
実行には`matplotlib`が必要です．

---

#### scale_instance.py
$python scale_instance.py _tsp-file-path_ _scale_

_tsp-file-path_ で指定したTSPインスタンスの，全都市の座標を _scale_ 倍します．

---

#### specify_city_num.py
$python specify_city_num.py _tsp-file-path_ _city-num_

_tsp-file-path_ で指定したTSPインスタンスの都市数が _city-num_ になるまで都市をランダムに削除します．
よって， _city-num_ で指定する値は，インスタンスの都市数よりも小さい必要があります．

ランダムに都市を削除する関係上，都市配置の均等性が崩れてしまいます．
そのため，このプログラムで都市を削除した後はもう一度TSPInstanceMakerで再配置を行ってください．
削除された都市数にもよりますが，大抵の場合は数世代で足りるはずです．

## LICENCE
opencv : Copyright(c) 2016, Itseez.

OpenCVTemplate : Copyright(c) 2016-2018, cafeunder.

## CONTACT
twitter : [@cafeunder](https://twitter.com/cafeunder)
