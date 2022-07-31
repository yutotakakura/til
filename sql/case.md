# CASE式

## GROUP BY句でCASE式を使うことで、集約単位となるコードや階級を柔軟に設定できる。これは非定性的な集計に大きな威力を発揮する。

### テーブル定義
```
CREATE TABLE PopTbl
(pref_name VARCHAR(32) PRIMARY KEY,
 population INTEGER NOT NULL);

INSERT INTO PopTbl VALUES('徳島', 100);
INSERT INTO PopTbl VALUES('香川', 200);
INSERT INTO PopTbl VALUES('愛媛', 150);
INSERT INTO PopTbl VALUES('高知', 200);
INSERT INTO PopTbl VALUES('福岡', 300);
INSERT INTO PopTbl VALUES('佐賀', 100);
INSERT INTO PopTbl VALUES('長崎', 200);
INSERT INTO PopTbl VALUES('東京', 400);
INSERT INTO PopTbl VALUES('群馬', 50);
```

### 条件
県名を地方名に再分類する

```
SELECT CASE pref_name
          WHEN '徳島' THEN '四国'
          WHEN '香川' THEN '四国'
          WHEN '愛媛' THEN '四国'
          WHEN '高知' THEN '四国'
          WHEN '福岡' THEN '九州'
          WHEN '佐賀' THEN '九州'
          WHEN '長崎' THEN '九州'
          ELSE 'その他' END AS district,
       SUM(population)
  FROM PopTbl
 GROUP BY CASE pref_name
            WHEN '徳島' THEN '四国'
            WHEN '香川' THEN '四国'
            WHEN '愛媛' THEN '四国'
            WHEN '高知' THEN '四国'
            WHEN '福岡' THEN '九州'
            WHEN '佐賀' THEN '九州'
            WHEN '長崎' THEN '九州'
            ELSE 'その他' END;
```

```
SELECT CASE pref_name
          WHEN '徳島' THEN '四国'
          WHEN '香川' THEN '四国'
          WHEN '愛媛' THEN '四国'
          WHEN '高知' THEN '四国'
          WHEN '福岡' THEN '九州'
          WHEN '佐賀' THEN '九州'
          WHEN '長崎' THEN '九州'
          ELSE 'その他' END AS district,
       SUM(population)
  FROM PopTbl
 GROUP BY district;
 -- districtでSELECT句でつけた別名をGROUP BYで参照している
 -- PostgreSQL, MySQLでは使える
```

### 条件
人口階級ごとに都道府県を分類する

```
SELECT CASE WHEN population < 100 THEN '01'
            WHEN population >= 100 AND population < 200 THEN '02'
            WHEN population >= 200 AND population < 300 THEN '03'
            WHEN population >= 300 THEN '04'
            ELSE NULL END AS pop_class,
       COUNT(*) AS cnt
  FROM PopTbl
 GROUP BY CASE WHEN population < 100 THEN '01'
               WHEN population >= 100 AND population < 200 THEN '02'
               WHEN population >= 200 AND population < 300 THEN '03'
               WHEN population >= 300 THEN '04'
               ELSE NULL END;
```

## 集約関数の中で使うことで、行持ちから列持ちへの水平展開も自由自在。

### テーブル定義
```
CREATE TABLE PopTbl2
(pref_name VARCHAR(32),
 sex CHAR(1) NOT NULL,
 population INTEGER NOT NULL,
    PRIMARY KEY(pref_name, sex));

INSERT INTO PopTbl2 VALUES('徳島', '1',	60 );
INSERT INTO PopTbl2 VALUES('徳島', '2',	40 );
INSERT INTO PopTbl2 VALUES('香川', '1',	100);
INSERT INTO PopTbl2 VALUES('香川', '2',	100);
INSERT INTO PopTbl2 VALUES('愛媛', '1',	100);
INSERT INTO PopTbl2 VALUES('愛媛', '2',	50 );
INSERT INTO PopTbl2 VALUES('高知', '1',	100);
INSERT INTO PopTbl2 VALUES('高知', '2',	100);
INSERT INTO PopTbl2 VALUES('福岡', '1',	100);
INSERT INTO PopTbl2 VALUES('福岡', '2',	200);
INSERT INTO PopTbl2 VALUES('佐賀', '1',	20 );
INSERT INTO PopTbl2 VALUES('佐賀', '2',	80 );
INSERT INTO PopTbl2 VALUES('長崎', '1',	125);
INSERT INTO PopTbl2 VALUES('長崎', '2',	125);
INSERT INTO PopTbl2 VALUES('東京', '1',	250);
INSERT INTO PopTbl2 VALUES('東京', '2',	150);
```

### 条件
性別ごとの人口を求める

### WHERE句を使った一般的なクエリ
```
-- 男性の人口
SELECT pref_name,
       population
  FROM PopTbl2
 WHERE sex = '1';
```

```
-- 女性の人口
SELECT pref_name,
       population
  FROM PopTbl2
 WHERE sex = '2';
```

### CASE式でまとめたクエリ
```
SELECT pref_name,
       -- 男性の人口
       SUM( CASE WHEN sex = '1' THEN population ELSE 0 END) AS cnt_m,
       -- 女性の人口
       SUM( CASE WHEN sex = '2' THEN population ELSE 0 END) AS cnt_f
  FROM PopTbl2
 GROUP BY pref_name;
```
WHERE句で条件分岐させるのは素人。プロはSELECT句で分岐させる。
ちなみにSUM関数なしだと以下のようにレコードが男性と女性で分かれる。同じ県で複数レコード出力される。
```
SELECT pref_name,
       -- 男性の人口
       CASE WHEN sex = '1' THEN population ELSE 0 END AS cnt_m,
       -- 女性の人口
       CASE WHEN sex = '2' THEN population ELSE 0 END AS cnt_f
  FROM PopTbl2;
```

## 反対に集約関数を条件式に組み込むことでHAVING句を使わずにクエリをまとめられる。

### テーブル定義
```
CREATE TABLE StudentClub
(std_id  INTEGER,
 club_id INTEGER,
 club_name VARCHAR(32),
 main_club_flg CHAR(1),
 PRIMARY KEY (std_id, club_id));

INSERT INTO StudentClub VALUES(100, 1, '野球', 'Y');
INSERT INTO StudentClub VALUES(100, 2, '吹奏楽', 'N');
INSERT INTO StudentClub VALUES(200, 2, '吹奏楽', 'N');
INSERT INTO StudentClub VALUES(200, 3, 'バドミントン', 'Y');
INSERT INTO StudentClub VALUES(200, 4, 'サッカー', 'N');
INSERT INTO StudentClub VALUES(300, 4, 'サッカー', 'N');
INSERT INTO StudentClub VALUES(400, 5, '水泳', 'N');
INSERT INTO StudentClub VALUES(500, 6, '囲碁', 'N');
```

### 条件
1つだけのクラブに所属している学生については、そのクラブIDを取得する。
複数のクラブに所属している学生については、主なクラブIDを取得する。

### HAVING句を使った一般的なクエリ
1つのクラブに専念している学生を選択
```
SELECT std_id, MAX(club_id) AS main_club
  FROM StudentClub
 GROUP BY std_id
HAVING COUNT(*) = 1;
```

クラブを掛け持ちしている学生を選択
```
SELECT std_id, club_id AS main_club
  FROM StudentClub
 WHERE main_club_flg = 'Y';
```

### CASE式でまとめたクエリ
```
SELECT std_id,
       CASE WHEN COUNT(*) = 1 --1つのクラブに専念する学生の場合
            THEN MAX(club_id)
            ELSE MAX(CASE WHEN main_club_flg = 'Y'
                          THEN club_id
                          ELSE NULL END) END AS main_club
  FROM StudentClub
 GROUP BY std_id;
 ```
 HAVING句で条件分岐させるのは素人。プロはSELECT句で分岐させる。

## ポイント
- 実装依存の関数より表現力が非常に強力な上、汎用性も高まって一石二鳥。
- そうした利点があるのも、CASE式が「文」ではなく「式」であるからこそ。
  - 式なので、実行時には評価されて一つの値に定まる。
  - CASE式は、列名や定数を書ける場所には常に書くことができる。
- CASE式を駆使することで複数のSQLを一つにまとめられ、可読性もパフォーマンスも向上していいこと尽くし。
