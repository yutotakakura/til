# ウィンドウ関数

## ポイント

- ウィンドウ関数の「ウィンドウ」とは、（原則として順序を持つ）「範囲」という意味。
- ウィンドウ関数の構文上では、PARTITION BY句と ORDER BY句で特徴づけられたレコードの集合を意味するが、一般的に簡略系の構文が使われるため、かえってウィンドウの存在を意識しにくい。
- PARTITION BY句はGROUP BY句から集約の機能を引いて、カットの機能だけを残し、ORDER BY句はレコードの順序を付ける。
- フレーム句はカーソルの機能をSQLに持ち込むことで、「カレントレコード」を中心にしたレコード集合の範囲を定義することができる。
- フレーム句を使うことで、異なる行のデータを1つの行に持ってくることができるようになり、行間比較が簡単に行えるようになった。
- ウィンドウ関数の内部動作としては、現在のところ、レコードのソートが行われている。将来的にハッシュが採用される可能性もゼロではない。

## ウィンドウとは何か

```
-- 無名ウィンドウ構文（原則こっちを使用）
SELECT shohin_id, shohin_mei, hanbai_tanka,
       AVG (hanbai_tanka) OVER (ORDER BY shohin_id                         
                                ROWS BETWEEN 2 PRECEDING                   
                                         AND CURRENT ROW) AS moving_avg    
  FROM Shohin;
```

```
-- 名前付きウィンドウ構文
SELECT shohin_id, shohin_mei, hanbai_tanka,
       AVG(hanbai_tanka) OVER W AS moving_avg
  FROM Shohin
WINDOW W AS (ORDER BY shohin_id
                 ROWS BETWEEN 2 PRECEDING 
                          AND CURRENT ROW);
```

```
-- 名前付きだとウィンドウ定義を使いまわせる
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

## ウィンドウ関数とは

### ウィンドウ関数の機能

- PARTITION BY句によるレコード集合のカット
- ORDER BY句によるレコード集合の順序付け
- フレーム句によるカレントレコードを中心としたサブセットの定義

## フレーム句を使って違う行を自分の行に持ってくる

```
-- テーブル定義
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

```
-- 各行について、過去の直近の日付を求める
SELECT sample_date AS cur_date,
       MIN(sample_date)
          OVER (ORDER BY sample_date ASC
                 ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING) AS latest_date
  FROM LoadSample;
```

```
-- 上記クエリに、load_valも追加
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

```
-- 名前ウィンドウ構文で上記クエリを一つにまとめる
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MIN(sample_date) OVER W AS latest_date,
       MIN(load_val) OVER W AS latest_load
  FROM LoadSample
WINDOW W AS (ORDER BY sample_date ASC
              ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING);
```

```
-- フレームを後ろに移動する
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MIN(sample_date) OVER W AS next_date,
       MIN(load_val) OVER W AS next_load
  FROM LoadSample
WINDOW W AS (ORDER BY sample_date ASC
                 ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING);
```

```
/*
MINの意味
AVGでもMAXでもSUMでも同じ。
集約させることに意味がある。
*/
SELECT sample_date AS cur_date,
       load_val AS cur_load,
       MAX(sample_date) OVER W AS latest_date,
       MAX(load_val) OVER W AS latest_load
  FROM LoadSample
WINDOW W AS (ORDER BY sample_date ASC
              ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING);
```

```
-- 「1日前」や「2日前」のように列の値に基づいたフレームを設定する
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


```
-- 行間比較を一般化する
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
# ウィンドウ関数で行間比較を行う

## ポイント

- 昔は、SQLで行同士の比較を行うときは、比較対象のテーブルを追加して、相関サブクエリを行なっていた。
- しかし相関サブクエリは、パフォーマンスと可読性が悪く、SQLユーザーからは不評だった。
- ウィンドウ関数という救世主の登場により、相関サブクエリを使う必要は無くなった。ウィンドウ関数は可読性が高く、コードも簡潔に記述できる。今後更なるパフォーマンス改善も見込める。

## 成長・後退・現状維持

```
-- テーブル定義
CREATE TABLE Sales
(year INTEGER NOT NULL , 
 sale INTEGER NOT NULL ,
 PRIMARY KEY (year));

INSERT INTO Sales VALUES (1990, 50);
INSERT INTO Sales VALUES (1991, 51);
INSERT INTO Sales VALUES (1992, 52);
INSERT INTO Sales VALUES (1993, 52);
INSERT INTO Sales VALUES (1994, 50);
INSERT INTO Sales VALUES (1995, 50);
INSERT INTO Sales VALUES (1996, 49);
INSERT INTO Sales VALUES (1997, 55);
```

```
-- 前年と年商が同じ年度を求める：相関サブクエリの使用
SELECT year,sale
  FROM Sales S1
 WHERE sale = (SELECT sale
                 FROM Sales S2
                WHERE S2.year = S1.year - 1)
 ORDER BY year;
```

```
-- 前年と年商が同じ年度を求める：ウィンドウ関数の使用
SELECT year, current_sale
  FROM (SELECT year,
               sale AS current_sale,
               SUM(sale) OVER (ORDER BY year
                               RANGE BETWEEN 1 PRECEDING
                                         AND 1 PRECEDING) AS pre_sale
          FROM Sales) TMP
 WHERE current_sale = pre_sale
 ORDER BY year;
```

```
-- ウィンドウ関数のみ切り出して実行するとわかりやすい
SELECT year,
       sale AS current_sale,
       SUM(sale) OVER (ORDER BY year
                       RANGE BETWEEN 1 PRECEDING
                                 AND 1 PRECEDING) AS pre_sale
  FROM Sales;
```

```
-- 成長・後退・現状維持を一度に求める：相関サブクエリを使用
SELECT year, current_sale AS sale,
       CASE WHEN current_sale = pre_sale
              THEN '現状維持'
            WHEN current_sale > pre_sale
              THEN '成長'
            WHEN current_sale < pre_sale
              THEN '衰退'
            ELSE '-' END AS var
  FROM (SELECT year,
               sale AS current_sale,
               (SELECT sale
                  FROM Sales S2
                 WHERE S2.year = S1.year - 1) AS pre_sale
          FROM Sales S1) TMP
 ORDER BY year;
```

```
-- 成長・後退・現状維持を一度に求める：ウィンドウ関数を使用
SELECT year, current_sale AS sale,
       CASE WHEN current_sale = pre_sale
             THEN '現状維持'
            WHEN current_sale > pre_sale
             THEN '成長'
            WHEN current_sale < pre_sale
             THEN '衰退'
            ELSE '-' END AS var
  FROM (SELECT year,
               sale AS current_sale,
               SUM(sale) OVER (ORDER BY year
                               RANGE BETWEEN 1 PRECEDING
                                         AND 1 PRECEDING) AS pre_sale
          FROM Sales) TMP
 ORDER BY year;
```

## 時系列に歯抜けがある場合：直近と比較

```
-- テーブル定義
CREATE TABLE Sales2
(year INTEGER NOT NULL , 
 sale INTEGER NOT NULL , 
 PRIMARY KEY (year));

INSERT INTO Sales2 VALUES (1990, 50);
-- 1991が歯抜け
INSERT INTO Sales2 VALUES (1992, 50);
INSERT INTO Sales2 VALUES (1993, 52);
INSERT INTO Sales2 VALUES (1994, 55);
-- 1995, 1996が歯抜け
INSERT INTO Sales2 VALUES (1997, 55);
```

```
-- 直近の年と同じ年商の年を選択する：相関サブクエリを使用
SELECT year, sale
  FROM Sales2 S1
 WHERE sale =
         (SELECT sale
            FROM Sales2 S2
           WHERE S2.year =
                  (SELECT MAX(year) -- 条件2 : 条件1を満たす年度の中で最大

# ウィンドウ関数で行間比較を行う

## ポイント

- 昔は、SQLで行同士の比較を行うときは、比較対象のテーブルを追加して、相関サブクエリを行なっていた。
- しかし相関サブクエリは、パフォーマンスと可読性が悪く、SQLユーザーからは不評だった。
- ウィンドウ関数という救世主の登場により、相関サブクエリを使う必要は無くなった。ウィンドウ関数は可読性が高く、コードも簡潔に記述できる。今後更なるパフォーマンス改善も見込める。

## 成長・後退・現状維持

```
-- テーブル定義
CREATE TABLE Sales
(year INTEGER NOT NULL , 
 sale INTEGER NOT NULL ,
 PRIMARY KEY (year));

INSERT INTO Sales VALUES (1990, 50);
INSERT INTO Sales VALUES (1991, 51);
INSERT INTO Sales VALUES (1992, 52);
INSERT INTO Sales VALUES (1993, 52);
INSERT INTO Sales VALUES (1994, 50);
INSERT INTO Sales VALUES (1995, 50);
INSERT INTO Sales VALUES (1996, 49);
INSERT INTO Sales VALUES (1997, 55);
```

```
-- 前年と年商が同じ年度を求める：相関サブクエリの使用
SELECT year,sale
  FROM Sales S1
 WHERE sale = (SELECT sale
                 FROM Sales S2
                WHERE S2.year = S1.year - 1)
 ORDER BY year;
```

```
-- 前年と年商が同じ年度を求める：ウィンドウ関数の使用
SELECT year, current_sale
  FROM (SELECT year,
               sale AS current_sale,
               SUM(sale) OVER (ORDER BY year
                               RANGE BETWEEN 1 PRECEDING
                                         AND 1 PRECEDING) AS pre_sale
          FROM Sales) TMP
 WHERE current_sale = pre_sale
 ORDER BY year;
```

```
-- ウィンドウ関数のみ切り出して実行するとわかりやすい
SELECT year,
       sale AS current_sale,
       SUM(sale) OVER (ORDER BY year
                       RANGE BETWEEN 1 PRECEDING
                                 AND 1 PRECEDING) AS pre_sale
  FROM Sales;
```

```
-- 成長・後退・現状維持を一度に求める：相関サブクエリを使用
SELECT year, current_sale AS sale,
       CASE WHEN current_sale = pre_sale
              THEN '現状維持'
            WHEN current_sale > pre_sale
              THEN '成長'
            WHEN current_sale < pre_sale
              THEN '衰退'
            ELSE '-' END AS var
  FROM (SELECT year,
               sale AS current_sale,
               (SELECT sale
                  FROM Sales S2
                 WHERE S2.year = S1.year - 1) AS pre_sale
          FROM Sales S1) TMP
 ORDER BY year;
```

```
-- 成長・後退・現状維持を一度に求める：ウィンドウ関数を使用
SELECT year, current_sale AS sale,
       CASE WHEN current_sale = pre_sale
             THEN '現状維持'
            WHEN current_sale > pre_sale
             THEN '成長'
            WHEN current_sale < pre_sale
             THEN '衰退'
            ELSE '-' END AS var
  FROM (SELECT year,
               sale AS current_sale,
               SUM(sale) OVER (ORDER BY year
                               RANGE BETWEEN 1 PRECEDING
                                         AND 1 PRECEDING) AS pre_sale
          FROM Sales) TMP
 ORDER BY year;
```

## 時系列に歯抜けがある場合：直近と比較

```
-- テーブル定義
CREATE TABLE Sales2
(year INTEGER NOT NULL , 
 sale INTEGER NOT NULL , 
 PRIMARY KEY (year));

INSERT INTO Sales2 VALUES (1990, 50);
-- 1991が歯抜け
INSERT INTO Sales2 VALUES (1992, 50);
INSERT INTO Sales2 VALUES (1993, 52);
INSERT INTO Sales2 VALUES (1994, 55);
-- 1995, 1996が歯抜け
INSERT INTO Sales2 VALUES (1997, 55);
```

```
-- 直近の年と同じ年商の年を選択する：相関サブクエリを使用
SELECT year, sale
  FROM Sales2 S1
 WHERE sale =
         (SELECT sale
            FROM Sales2 S2
           WHERE S2.year =
                  (SELECT MAX(year) -- 条件2 : 条件1を満たす年度の中で最大
                     FROM Sales2 S3
                    WHERE S1.year > S3.year)) -- 条件1 : 自分より過去である
 ORDER BY year;
```

```
-- 直近の年と同じ年商の年を選択する：ウィンドウ関数を使用
SELECT year, current_sale
  FROM (SELECT year,
               sale AS current_sale,
               SUM(sale) OVER (ORDER BY year
                                ROWS BETWEEN 1 PRECEDING
                                         AND 1 PRECEDING) AS pre_sale
          FROM Sales2) TMP
         WHERE current_sale = pre_sale
 ORDER BY year;
```

## オーバーラップする期間を調べる

```
-- テーブル定義
CREATE TABLE Reservations
(reserver    VARCHAR(30) PRIMARY KEY,
 start_date  DATE  NOT NULL,
 end_date    DATE  NOT NULL);

INSERT INTO Reservations VALUES('木村', '2018-10-26', '2018-10-27');
INSERT INTO Reservations VALUES('荒木', '2018-10-28', '2018-10-31');
INSERT INTO Reservations VALUES('堀', '2018-10-31', '2018-11-01');
INSERT INTO Reservations VALUES('山本', '2018-11-03', '2018-11-04');
INSERT INTO Reservations VALUES('内田', '2018-11-03', '2018-11-05');
INSERT INTO Reservations VALUES('水谷', '2018-11-06', '2018-11-06');
```

```
-- オーバーラップする期間を調べる：相関サブクエリの使用
SELECT reserver, start_date, end_date
  FROM Reservations R1
 WHERE EXISTS
        (SELECT *
           FROM Reservations R2
          WHERE R1.reserver <> R2.reserver -- 自分以外の客と比較する
            AND ( R1.start_date BETWEEN R2.start_date AND R2.end_date
                   -- 条件1 : 開始日が他の期間内にある
               OR R1.end_date BETWEEN R2.start_date AND R2.end_date));
                   -- 条件2 : 終了日が他の期間内にある
```

```
-- オーバーラップする期間を調べる：ウィンドウ関数の使用
SELECT reserver, next_reserver
  FROM (SELECT reserver,
               start_date,
               end_date,
               MAX(start_date) OVER (ORDER BY start_date
                                      ROWS BETWEEN 1 FOLLOWING 
                                               AND 1 FOLLOWING) AS next_start_date,
               MAX(reserver)   OVER (ORDER BY start_date
                                      ROWS BETWEEN 1 FOLLOWING 
                                               AND 1 FOLLOWING) AS next_reserver
          FROM Reservations) TMP
 WHERE next_start_date BETWEEN start_date AND end_date;
 ```
                     FROM Sales2 S3
                    WHERE S1.year > S3.year)) -- 条件1 : 自分より過去である
 ORDER BY year;
```

```
-- 直近の年と同じ年商の年を選択する：ウィンドウ関数を使用
SELECT year, current_sale
  FROM (SELECT year,
               sale AS current_sale,
               SUM(sale) OVER (ORDER BY year
                                ROWS BETWEEN 1 PRECEDING
                                         AND 1 PRECEDING) AS pre_sale
          FROM Sales2) TMP
         WHERE current_sale = pre_sale
 ORDER BY year;
```

## オーバーラップする期間を調べる

```
-- テーブル定義
CREATE TABLE Reservations
(reserver    VARCHAR(30) PRIMARY KEY,
 start_date  DATE  NOT NULL,
 end_date    DATE  NOT NULL);

INSERT INTO Reservations VALUES('木村', '2018-10-26', '2018-10-27');
INSERT INTO Reservations VALUES('荒木', '2018-10-28', '2018-10-31');
INSERT INTO Reservations VALUES('堀', '2018-10-31', '2018-11-01');
INSERT INTO Reservations VALUES('山本', '2018-11-03', '2018-11-04');
INSERT INTO Reservations VALUES('内田', '2018-11-03', '2018-11-05');
INSERT INTO Reservations VALUES('水谷', '2018-11-06', '2018-11-06');
```

```
-- オーバーラップする期間を調べる：相関サブクエリの使用
SELECT reserver, start_date, end_date
  FROM Reservations R1
 WHERE EXISTS
        (SELECT *
           FROM Reservations R2
          WHERE R1.reserver <> R2.reserver -- 自分以外の客と比較する
            AND ( R1.start_date BETWEEN R2.start_date AND R2.end_date
                   -- 条件1 : 開始日が他の期間内にある
               OR R1.end_date BETWEEN R2.start_date AND R2.end_date));
                   -- 条件2 : 終了日が他の期間内にある
```

```
-- オーバーラップする期間を調べる：ウィンドウ関数の使用
SELECT reserver, next_reserver
  FROM (SELECT reserver,
               start_date,
               end_date,
               MAX(start_date) OVER (ORDER BY start_date
                                      ROWS BETWEEN 1 FOLLOWING 
                                               AND 1 FOLLOWING) AS next_start_date,
               MAX(reserver)   OVER (ORDER BY start_date
                                      ROWS BETWEEN 1 FOLLOWING 
                                               AND 1 FOLLOWING) AS next_reserver
          FROM Reservations) TMP
 WHERE next_start_date BETWEEN start_date AND end_date;
 ```
