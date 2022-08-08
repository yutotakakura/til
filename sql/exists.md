# EXISTS述語の使い方

## ポイント
- SQLにおける述語とは、真理値を返す関数のこと。
- EXISTS句だけが、他の述語と違って（行の）集合を引数に渡せる。
- その点で、EXISTSは高階関数の一種と見なせる。
- SQLには全称量化子に相当する演算子がないので、NOT EXISTSで代用する。

## 理論編
#### 述語とは何か
述語とは、戻り値が真理値になる関数。<br>
テーブルは、行の集合ではなく、*命題の集合*とも取れる。

| name | sex | age |
| --- | --- | --- |
| 田中 | 男| 28 |
| 鈴木 | 女 | 21 |
| 山田 | 男 | 32 |

田中さんは、性別は男であり、かつ、年齢が28歳である。

#### 存在の階層
一階の述語：一行を入力する述語<br>
ニ階の述語：集合を入力する述語<br>
三階の述語：集合の集合を入力する述語<br>
...<br>
SQLには三階以上の述語は登場しない。行までを入力に受け付けられて、テーブルは入力に受け付けられない。<br>
*EXISTSは高階関数である。*

### 全称量化と存在量化
述語論理には、量子化という特別な述語がある。
- 全称量化子：すべてのxが条件Pを満たす。
- 存在量子化：条件Pを満たすxが（少なくとも一つ）存在する。

*SQLは全称量化子に対応する述語を導入しなかったので、存在量子化に書き換える必要がある。*

## 実践編
### テーブルに存在「しない」データを探す
テーブル定義
```
CREATE TABLE Meetings
(meeting CHAR(32) NOT NULL,
 person  CHAR(32) NOT NULL,
 PRIMARY KEY (meeting, person));

INSERT INTO Meetings VALUES('第1回', '伊藤');
INSERT INTO Meetings VALUES('第1回', '水島');
INSERT INTO Meetings VALUES('第1回', '坂東');
INSERT INTO Meetings VALUES('第2回', '伊藤');
INSERT INTO Meetings VALUES('第2回', '宮田');
INSERT INTO Meetings VALUES('第3回', '坂東');
INSERT INTO Meetings VALUES('第3回', '水島');
INSERT INTO Meetings VALUES('第3回', '宮田');
```
「これこれこういう性質を満たす」ではなく、「データが存在するか否か」という、次数の一つ高い問題設定　→ 二階の問い合わせ<br>

全員が皆勤出席だった場合
```
SELECT DISTINCT M1.meeting, M2.person
  FROM Meetings M1 CROSS JOIN Meetings M2;
```

欠席者だけを求めるクエリ：存在量化の応用
```
SELECT DISTINCT M1.meeting, M2.person
  FROM Meetings M1 CROSS JOIN Meetings M2
 WHERE NOT EXISTS
        (SELECT *
           FROM Meetings M3
          WHERE M1.meeting = M3.meeting
            AND M2.person = M3.person);
```

欠席者だけを求めるクエリ：差集合演算の利用<br>
EXCEPT：複数のSELECTの結果の差異を取得する。<br>
今回なら、重複順列（総当たり）と元データを比較して重複順列にしかないものだけを選択している。<br>
EXCEPT は SQL SERVER だけでしか使えない。
```
SELECT M1.meeting, M2.person
  FROM Meetings M1, Meetings M2
EXCEPT
SELECT meeting, person
  FROM Meetings;
```

### 肯定　<=> の変換に慣れる
テーブル定義
```
CREATE TABLE TestScores
(student_id INTEGER,
 subject    VARCHAR(32) ,
 score      INTEGER,
  PRIMARY KEY(student_id, subject));

INSERT INTO TestScores VALUES(100, '算数',100);
INSERT INTO TestScores VALUES(100, '国語',80);
INSERT INTO TestScores VALUES(100, '理科',80);
INSERT INTO TestScores VALUES(200, '算数',80);
INSERT INTO TestScores VALUES(200, '国語',95);
INSERT INTO TestScores VALUES(300, '算数',40);
INSERT INTO TestScores VALUES(300, '国語',90);
INSERT INTO TestScores VALUES(300, '社会',55);
INSERT INTO TestScores VALUES(400, '算数',80);
```
全ての教科が50点以上である生徒を選択する。<br>
「50点未満の教科が1つも存在しない」と言い換える。
```
SELECT DISTINCT student_id
  FROM TestScores TS1
 WHERE NOT EXISTS -- 以下の条件を満たす行が存在しない
        (SELECT *
           FROM TestScores TS2
          WHERE TS2.student_id = TS1.student_id
            AND TS2.score < 50); -- 50点未満の教科
```
算数の点数が80点以上、かつ、国語の点数が50点以上の生徒を選択する。<br>
これは「ある学生の全ての行について、教科が算数ならば80点以上であり、教科が国語であれば50点以上である。」という全称量化文である。
```
SELECT DISTINCT student_id
  FROM TestScores TS1
 WHERE subject IN ('算数', '国語')
   AND NOT EXISTS
        (SELECT *
           FROM TestScores TS2
          WHERE TS2.student_id = TS1.student_id
            AND 1 = CASE WHEN subject = '算数' AND score < 80 THEN 1
                         WHEN subject = '国語' AND score < 50 THEN 1
                         ELSE 0 END);
```
国語のデータが存在しない400番の生徒を結果から除外する。
```
SELECT student_id
  FROM TestScores TS1
 WHERE subject IN ('算数', '国語')
   AND NOT EXISTS
        (SELECT *
           FROM TestScores TS2
          WHERE TS2.student_id = TS1.student_id
            AND 1 = CASE WHEN subject = '算数' AND score < 80 THEN 1
                         WHEN subject = '国語' AND score < 50 THEN 1
                         ELSE 0 END)
 GROUP BY student_id
HAVING COUNT(*) = 2;
```

### 集合と述語の比較(HAVING と NOT EXISTS)
テーブル定義
```
CREATE TABLE Projects
(project_id VARCHAR(32),
 step_nbr   INTEGER ,
 status     VARCHAR(32),
  PRIMARY KEY(project_id, step_nbr));

INSERT INTO Projects VALUES('AA100', 0, '完了');
INSERT INTO Projects VALUES('AA100', 1, '待機');
INSERT INTO Projects VALUES('AA100', 2, '待機');
INSERT INTO Projects VALUES('B200',  0, '待機');
INSERT INTO Projects VALUES('B200',  1, '待機');
INSERT INTO Projects VALUES('CS300', 0, '完了');
INSERT INTO Projects VALUES('CS300', 1, '完了');
INSERT INTO Projects VALUES('CS300', 2, '待機');
INSERT INTO Projects VALUES('CS300', 3, '待機');
INSERT INTO Projects VALUES('DY400', 0, '完了');
INSERT INTO Projects VALUES('DY400', 1, '完了');
INSERT INTO Projects VALUES('DY400', 2, '完了');
```
工程1番まで完了のプロジェクトを選択：集合指向的な解答
```
SELECT project_id
  FROM Projects
 GROUP BY project_id
HAVING COUNT(*) = SUM(CASE WHEN step_nbr <= 1 AND status = '完了' THEN 1
                           WHEN step_nbr > 1 AND status = '待機' THEN 1
                           ELSE 0 END);
```
「プロジェクト内の全ての行について、工程番号が1以下ならば完了であり、1より大きければ待機である。」という全称文であり、これを量子文的に置き換える。
```
SELECT *
  FROM Projects P1
 WHERE NOT EXISTS
        (SELECT status
           FROM Projects P2
          WHERE P1.project_id = P2.project_id -- プロジェクトごとに条件を調べる
            AND status <> CASE WHEN step_nbr <= 1 -- 全称文を二重否定で表現する
                               THEN '完了'
                               ELSE '待機' END);
```
NOT EXISTS の特徴
- HAVING に比べて二重否定を使う分可読性が悪い
- パフォーマンスはいい
  - 1行でも条件を満たさない行があれば検索を断ち切る
  - 結合条件でproject_idのインデックスを利用できる
- 結果に含めれられる情報量が多い
  - HAVING句では集約されるのでproject_idしかわからない

### オール1の行を捜す
テーブル定義(悪い例だけど)
```
CREATE TABLE ArrayTbl
 (keycol CHAR(1) PRIMARY KEY,
  col1  INTEGER,
  col2  INTEGER,
  col3  INTEGER,
  col4  INTEGER,
  col5  INTEGER,
  col6  INTEGER,
  col7  INTEGER,
  col8  INTEGER,
  col9  INTEGER,
  col10 INTEGER);

INSERT INTO ArrayTbl VALUES('A', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO ArrayTbl VALUES('B', 3, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO ArrayTbl VALUES('C', 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
INSERT INTO ArrayTbl VALUES('D', NULL, NULL, 9, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO ArrayTbl VALUES('E', NULL, 3, NULL, 1, 9, NULL, NULL, 9, NULL, NULL);
```
列はある程度持続的な構造として考えるべきである。<br>
要件
- 「オール1」の行を探したい
- 「少なくとも1つは9」の行を探したい
列方向への全称量化<br>
MySQLでは、ALLやANY述語の引数をサブクエリに限定しているため、以下のクエリは構文エラーとなる。(PostgreSQLではOK)
```
SELECT *
  FROM ArrayTbl
WHERE 1 = ALL (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);
```
```
SELECT *
  FROM ArrayTbl
 WHERE 9 = ANY (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);
```
同義
```
SELECT *
  FROM ArrayTbl
 WHERE 9 IN (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);
```
オールNULLの行を探す<br>
間違ったクエリ
```
SELECT *
  FROM ArrayTbl
 WHERE NULL = ALL (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);
```
正しいクエリ
```
SELECT *
  FROM ArrayTbl
 WHERE COALESCE(col1, col2, col3, col4, col5, col6, col7, col8, col9, col10) IS NULL;
```