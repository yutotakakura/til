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
...
SQLには三階以上の述語は登場しない。行までを入力に受け付けられて、テーブルは入力に受け付けられない。<br>
*EXISTSは高階関数である。*

### 全称量化と存在量化
述語論理には、量子化という特別な述語がある。
- 全称量化子：すべてのxが条件Pを満たす。
- 存在量子化：条件Pを満たすxが（少なくとも一つ）存在する。
*SQLは全称量化子に対応する述語を導入しなかったので、存在量子化に書き換える必要がある。*<br>

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
--åáê»é“ÇæÇØÇãÅÇﬂÇÈÉNÉGÉäÅFÇªÇÃ1Å@ë∂ç›ó âªÇÃâûóp
SELECT DISTINCT M1.meeting, M2.person
  FROM Meetings M1 CROSS JOIN Meetings M2
 WHERE NOT EXISTS
        (SELECT *
           FROM Meetings M3
          WHERE M1.meeting = M3.meeting
            AND M2.person = M3.person);


--åáê»é“ÇæÇØÇãÅÇﬂÇÈÉNÉGÉäÅFÇªÇÃ2Å@ç∑èWçáââéZÇÃóòóp
SELECT M1.meeting, M2.person
  FROM Meetings M1, Meetings M2
EXCEPT
SELECT meeting, person
  FROM Meetings;


/* ëSèÃó âªÅ@ÇªÇÃÇPÅFçmíËÅÃìÒèdî€íËÇÃïœä∑Ç…äµÇÍÇÊÇ§ */
CREATE TABLE TestScores
(student_id INTEGER,
 subject    VARCHAR(32) ,
 score      INTEGER,
  PRIMARY KEY(student_id, subject));

INSERT INTO TestScores VALUES(100, 'éZêî',100);
INSERT INTO TestScores VALUES(100, 'çëåÍ',80);
INSERT INTO TestScores VALUES(100, 'óùâ»',80);
INSERT INTO TestScores VALUES(200, 'éZêî',80);
INSERT INTO TestScores VALUES(200, 'çëåÍ',95);
INSERT INTO TestScores VALUES(300, 'éZêî',40);
INSERT INTO TestScores VALUES(300, 'çëåÍ',90);
INSERT INTO TestScores VALUES(300, 'é–âÔ',55);
INSERT INTO TestScores VALUES(400, 'éZêî',80);


SELECT DISTINCT student_id
  FROM TestScores TS1
 WHERE NOT EXISTS -- à»â∫ÇÃèåèÇñûÇΩÇ∑çsÇ™ë∂ç›ÇµÇ»Ç¢
        (SELECT *
           FROM TestScores TS2
          WHERE TS2.student_id = TS1.student_id
            AND TS2.score < 50); --50 ì_ñ¢ñûÇÃã≥â»


SELECT DISTINCT student_id
  FROM TestScores TS1
 WHERE subject IN ('éZêî', 'çëåÍ')
   AND NOT EXISTS
        (SELECT *
           FROM TestScores TS2
          WHERE TS2.student_id = TS1.student_id
            AND 1 = CASE WHEN subject = 'éZêî' AND score < 80 THEN 1
                         WHEN subject = 'çëåÍ' AND score < 50 THEN 1
                         ELSE 0 END);


SELECT student_id
  FROM TestScores TS1
 WHERE subject IN ('éZêî', 'çëåÍ')
   AND NOT EXISTS
        (SELECT *
           FROM TestScores TS2
          WHERE TS2.student_id = TS1.student_id
            AND 1 = CASE WHEN subject = 'éZêî' AND score < 80 THEN 1
                         WHEN subject = 'çëåÍ' AND score < 50 THEN 1
                         ELSE 0 END)
 GROUP BY student_id
HAVING COUNT(*) = 2;


/* ëSèÃó âªÅ@ÇªÇÃÇQÅFèWçáVS èqåÍÅ\Å\ê¶Ç¢ÇÃÇÕÇ«Ç¡ÇøÇæÅH */
CREATE TABLE Projects
(project_id VARCHAR(32),
 step_nbr   INTEGER ,
 status     VARCHAR(32),
  PRIMARY KEY(project_id, step_nbr));

INSERT INTO Projects VALUES('AA100', 0, 'äÆóπ');
INSERT INTO Projects VALUES('AA100', 1, 'ë“ã@');
INSERT INTO Projects VALUES('AA100', 2, 'ë“ã@');
INSERT INTO Projects VALUES('B200',  0, 'ë“ã@');
INSERT INTO Projects VALUES('B200',  1, 'ë“ã@');
INSERT INTO Projects VALUES('CS300', 0, 'äÆóπ');
INSERT INTO Projects VALUES('CS300', 1, 'äÆóπ');
INSERT INTO Projects VALUES('CS300', 2, 'ë“ã@');
INSERT INTO Projects VALUES('CS300', 3, 'ë“ã@');
INSERT INTO Projects VALUES('DY400', 0, 'äÆóπ');
INSERT INTO Projects VALUES('DY400', 1, 'äÆóπ');
INSERT INTO Projects VALUES('DY400', 2, 'äÆóπ');


--çHíˆ1î‘Ç‹Ç≈äÆóπÇÃÉvÉçÉWÉFÉNÉgÇëIëÅFèWçáéwå¸ìIÇ»âìö
SELECT project_id
  FROM Projects
 GROUP BY project_id
HAVING COUNT(*) = SUM(CASE WHEN step_nbr <= 1 AND status = 'äÆóπ' THEN 1
                           WHEN step_nbr > 1 AND status = 'ë“ã@' THEN 1
                           ELSE 0 END);


--çHíˆ1î‘Ç‹Ç≈äÆóπÇÃÉvÉçÉWÉFÉNÉgÇëIëÅFèqåÍò_óùìIÇ»âìö
SELECT *
  FROM Projects P1
 WHERE NOT EXISTS
        (SELECT status
           FROM Projects P2
          WHERE P1.project_id = P2. project_id --ÉvÉçÉWÉFÉNÉgÇ≤Ç∆Ç…èåèÇí≤Ç◊ÇÈ
            AND status <> CASE WHEN step_nbr <= 1 --ëSèÃï∂ÇìÒèdî€íËÇ≈ï\åªÇ∑ÇÈ
                               THEN 'äÆóπ'
                               ELSE 'ë“ã@' END);


/* óÒÇ…ëŒÇ∑ÇÈó âªÅFÉIÅ[ÉãÇPÇÃçsÇíTÇπ */
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

--ÉIÅ[ÉãNULL
INSERT INTO ArrayTbl VALUES('A', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO ArrayTbl VALUES('B', 3, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
--ÉIÅ[Éã1
INSERT INTO ArrayTbl VALUES('C', 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
--è≠Ç»Ç≠Ç∆Ç‡àÍÇ¬ÇÕ9
INSERT INTO ArrayTbl VALUES('D', NULL, NULL, 9, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO ArrayTbl VALUES('E', NULL, 3, NULL, 1, 9, NULL, NULL, 9, NULL, NULL);


--óÒï˚å¸Ç÷ÇÃëSèÃó âªÅFå|ÇÃÇ†ÇÈìöÇ¶
SELECT *
  FROM ArrayTbl
WHERE 1 = ALL (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);

SELECT *
  FROM ArrayTbl
 WHERE 9 = ANY (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);

SELECT *
  FROM ArrayTbl
 WHERE 9 IN (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);

--ÉIÅ[ÉãNULLÇÃçsÇíTÇ∑ÅFä‘à·Ç¡ÇΩìöÇ¶
SELECT *
  FROM ArrayTbl
 WHERE NULL = ALL (col1, col2, col3, col4, col5, col6, col7, col8, col9, col10);

--ÉIÅ[ÉãNULLÇÃçsÇíTÇ∑ÅFê≥ÇµÇ¢ìöÇ¶
SELECT *
  FROM ArrayTbl
 WHERE COALESCE(col1, col2, col3, col4, col5, col6, col7, col8, col9, col10) IS NULL;


ââèKñ‚ëË
/* 5-1ÅFîzóÒÉeÅ[ÉuÉãÅ\Å\çséùÇøÇÃèÍçá */
CREATE TABLE ArrayTbl2
 (key   CHAR(1) NOT NULL,
    i   INTEGER NOT NULL,
  val   INTEGER,
  PRIMARY KEY (key, i));

/* AÇÕÉIÅ[ÉãNULLÅABÇÕàÍÇ¬ÇæÇØîÒNULLÅACÇÕÉIÅ[ÉãîÒNULL */
INSERT INTO ArrayTbl2 VALUES('A', 1, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 2, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 3, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 4, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 5, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 6, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 7, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 8, NULL);
INSERT INTO ArrayTbl2 VALUES('A', 9, NULL);
INSERT INTO ArrayTbl2 VALUES('A',10, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 1, 3);
INSERT INTO ArrayTbl2 VALUES('B', 2, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 3, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 4, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 5, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 6, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 7, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 8, NULL);
INSERT INTO ArrayTbl2 VALUES('B', 9, NULL);
INSERT INTO ArrayTbl2 VALUES('B',10, NULL);
INSERT INTO ArrayTbl2 VALUES('C', 1, 1);
INSERT INTO ArrayTbl2 VALUES('C', 2, 1);
INSERT INTO ArrayTbl2 VALUES('C', 3, 1);
INSERT INTO ArrayTbl2 VALUES('C', 4, 1);
INSERT INTO ArrayTbl2 VALUES('C', 5, 1);
INSERT INTO ArrayTbl2 VALUES('C', 6, 1);
INSERT INTO ArrayTbl2 VALUES('C', 7, 1);
INSERT INTO ArrayTbl2 VALUES('C', 8, 1);
INSERT INTO ArrayTbl2 VALUES('C', 9, 1);
INSERT INTO ArrayTbl2 VALUES('C',10, 1);
```