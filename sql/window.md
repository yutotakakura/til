# ウィンドウ関数

## ウィンドウ関数の「ウィンドウ」とは、（原則として順序を持つ）「範囲」という意味

## ウィンドウ関数の構文上では、PARTITION BY句と ORDER BY句で特徴づけられたレコードの集合を意味するが、一般的に簡略系の構文が使われるため、かえってウィンドウの存在を意識しにくい。

### 無名ウィンドウ構文（原則こっちを使用）
```
SELECT shohin_id, shohin_mei, hanbai_tanka,
       AVG (hanbai_tanka) OVER (ORDER BY shohin_id                         
                                ROWS BETWEEN 2 PRECEDING                   
                                         AND CURRENT ROW) AS moving_avg    
  FROM Shohin;
```

### 名前付きウィンドウ構文
```
SELECT shohin_id, shohin_mei, hanbai_tanka,
       AVG(hanbai_tanka) OVER W AS moving_avg
  FROM Shohin
WINDOW W AS (ORDER BY shohin_id
                 ROWS BETWEEN 2 PRECEDING 
                          AND CURRENT ROW);
```
名前付きだとウィンドウ定義を使いまわせる
```
SELECT shohin_id, shohin_mei, hanbai_tanka,
       AVG(hanbai_tanka)   OVER W AS moving_avg,
       SUM(hanbai_tanka)   OVER W AS moving_sum,
       COUNT(hanbai_tanka) OVER W AS moving_count,
       MAX(hanbai_tanka)   OVER W AS moving_max
  FROM Shohin
WINDOW W AS (ORDER BY shohin_id
                 ROWS BETWEEN 2 PRECEDING 
                          AND CURRENT ROW);
```

## PARTITION BY句はGROUP BY句から集約の機能を引いて、カットの機能だけを残し、ORDER BY句はレコードの順序を付ける。

### ウィンドウ関数の機能
- PARTITION BY句によるレコード集合のカット
- ORDER BY句によるレコード集合の順序付け
- フレーム句によるカレントレコードを中心としたサブセットの定義

## フレーム句はカーソルの機能をSQLに持ち込むことで、「カレントレコード」を中心にしたレコード集合の範囲を定義することができる。

### テーブル定義
```
CREATE TABLE LoadSample
(sample_date   DATE PRIMARY KEY,
 load_val      INTEGER NOT NULL);

INSERT INTO LoadSample VALUES('2018-02-01',   1024);
INSERT INTO LoadSample VALUES('2018-02-02',   2366);
INSERT INTO LoadSample VALUES('2018-02-05',   2366);
INSERT INTO LoadSample VALUES('2018-02-07',    985);
INSERT INTO LoadSample VALUES('2018-02-08',    780);
INSERT INTO LoadSample VALUES('2018-02-12',   1000);
```

### 各行について、過去の直近の日付を求める
```
SELECT sample_date AS cur_date,
       MIN(sample_date)
          OVER (ORDER BY sample_date ASC
                 ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING) AS latest_date
  FROM LoadSample;
```

### 上記クエリに、load_valも追加
```
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MIN(sample_date)
          OVER (ORDER BY sample_date ASC
                 ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING) AS latest_date,
       MIN(load_val)
          OVER (ORDER BY sample_date ASC
                 ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING) AS latest_load
  FROM LoadSample;
```

### 名前ウィンドウ構文で上記クエリを一つにまとめる
```
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MIN(sample_date) OVER W AS latest_date,
       MIN(load_val) OVER W AS latest_load
  FROM LoadSample
WINDOW W AS (ORDER BY sample_date ASC
              ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING);
```

### フレームを後ろに移動する
```
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MIN(sample_date) OVER W AS next_date,
       MIN(load_val) OVER W AS next_load
  FROM LoadSample
WINDOW W AS (ORDER BY sample_date ASC
                 ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING);
```

### MINの意味
AVGでもMAXでもSUMでも同じ。
集約させることに意味がある。
```
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MAX(sample_date) OVER W AS latest_date,
       MAX(load_val) OVER W AS latest_load
  FROM LoadSample
WINDOW W AS (ORDER BY sample_date ASC
              ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING);
```

### 「1日前」や「2日前」のように列の値に基づいたフレームを設定する
```
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MIN(sample_date)
          OVER (ORDER BY sample_date ASC
                RANGE BETWEEN interval '1' day PRECEDING
                          AND interval '1' day PRECEDING
                ) AS day1_before,
       MIN(load_val)
          OVER (ORDER BY sample_date ASC
                RANGE BETWEEN interval '1' day PRECEDING
                          AND interval '1' day PRECEDING
                ) AS load_day1_before
  FROM LoadSample;
```

### 行間比較を一般化する
```
SELECT sample_date AS cur_date,
       MIN(sample_date)
          OVER (ORDER BY sample_date ASC
                 ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING) AS latest_1,
       MIN(sample_date)
          OVER (ORDER BY sample_date ASC
                 ROWS BETWEEN 2 PRECEDING AND 2 PRECEDING) AS latest_2,
       MIN(sample_date)
          OVER (ORDER BY sample_date ASC
                 ROWS BETWEEN 3 PRECEDING AND 3 PRECEDING) AS latest_3
  FROM LoadSample;
```

フレーム句を使うことで、異なる行のデータを1つの行に持ってくることができるようになり、行間比較が簡単に行えるようになった。

## ウィンドウ関数の内部動作としては、現在のところ、レコードのソートが行われている。将来的にハッシュが採用される可能性もゼロではない。