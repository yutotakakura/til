# HAVING句の力

## ポイント

- テーブルはファイルではない。行も順序を持たない。そのためSQLでは"原則"ソートを記述しない。
- 代わりにSQLは、求める集合に辿り着くまで次々に集合を作る。SQLで考えるときは四角と矢印を描くのではなく、円を描くのがコツ。
- WHERE句が集合の要素の性質を調べつ道具であるのに対し、HAVING句は集合自身の性質を調べる道具である。
- SQLで検索条件を設定するときは、検索対象となる実体が集合なのか集合の要素なのかを見極めることが基本。
    - 実体1つにつき1行が対応している　→ 要素なのでWHERE句を使う
    - 実体1つにつき複数行が対応している　→ 集合なのでHAVING句を使う

## データの歯抜けを探す

```
-- テーブル定義
CREATE TABLE SeqTbl
(seq  INTEGER PRIMARY KEY,
 name VARCHAR(16) NOT NULL);

INSERT INTO SeqTbl VALUES(1, 'ディック');
INSERT INTO SeqTbl VALUES(2, 'アン');
INSERT INTO SeqTbl VALUES(3, 'ライル');
INSERT INTO SeqTbl VALUES(5, 'カー');
INSERT INTO SeqTbl VALUES(6, 'マリー');
INSERT INTO SeqTbl VALUES(8, 'ベン');
```

- テーブルがファイルなら、昇順か降順にソートさせてループさせて次の値と比較すればいい。
- ファイルのレコードは順序をもつが、テーブルの行は順序を持たないため、集合として扱う。

### 結果が返れば歯抜けあり

```
-- 数列の連続性のみ調べる。
SELECT '歯抜けあり' AS gap 
  FROM SeqTbl 
HAVING COUNT(*) <> MAX(seq);
```

- 2つの集合に一対一対応があるか調べている。
- HAVING句はGROUP BYがなくても使える。

### 歯抜けの最小値を探す。

```
SELECT MIN(seq + 1) AS gap 
  FROM SeqTbl 
 WHERE (seq+ 1) NOT IN (SELECT seq FROM SeqTbl);
```
- 自分よりも1大きいseqが存在するかどうかを調べる。
- NULLがあったら、もちろん正しい挙動はしない。

### 欠番を調べる（発展版）

```
-- テーブル定義
CREATE TABLE SeqTbl
( seq INTEGER NOT NULL PRIMARY KEY);
```

```
-- ケース1：欠番なし（開始値＝1）
DELETE FROM SeqTbl;
INSERT INTO SeqTbl VALUES(1);
INSERT INTO SeqTbl VALUES(2);
INSERT INTO SeqTbl VALUES(3);
INSERT INTO SeqTbl VALUES(4);
INSERT INTO SeqTbl VALUES(5);
```

```
-- ケース2：欠番あり（開始値＝1）
DELETE FROM SeqTbl;
INSERT INTO SeqTbl VALUES(1);
INSERT INTO SeqTbl VALUES(2);
INSERT INTO SeqTbl VALUES(4);
INSERT INTO SeqTbl VALUES(5);
INSERT INTO SeqTbl VALUES(8);
```

```
-- ケース3：欠番なし（開始値<>1）
DELETE FROM SeqTbl;
INSERT INTO SeqTbl VALUES(3);
INSERT INTO SeqTbl VALUES(4);
INSERT INTO SeqTbl VALUES(5);
INSERT INTO SeqTbl VALUES(6);
INSERT INTO SeqTbl VALUES(7);
```

```
-- ケース4：欠番あり（開始値<>1）
DELETE FROM SeqTbl;
INSERT INTO SeqTbl VALUES(3);
INSERT INTO SeqTbl VALUES(4);
INSERT INTO SeqTbl VALUES(7);
INSERT INTO SeqTbl VALUES(8);
INSERT INTO SeqTbl VALUES(10);
```

```
-- 欠番がない場合、「上限値 - 下限値 + 1」とすれば一般化できる。
SELECT '歯抜けあり' AS gap 
  FROM SeqTbl 
HAVING COUNT(*) <> MAX(seq) - MIN(seq) + 1;
```

```
-- 欠番があってもなくても一行返す。
SELECT CASE WHEN COUNT(*) = 0 THEN 'テーブルが空です。' 
            WHEN COUNT(*) <> MAX(seq) -MIN(seq) + 1 THEN '歯抜けあり' 
            ELSE '連続' END AS gap 
  FROM SeqTbl;
```

```
-- 歯抜けの最小値を探す：テーブルに1がない場合は、1を返す。
SELECT CASE WHEN COUNT(*) = 0 OR MIN(seq)  >  1 -- 下限が1でない場合、1を返す。 
              THEN 1  
              ELSE (SELECT MIN(seq +1)  -- 下限が1の場合、最小の欠番を返す。
                       FROM SeqTbl S1  
                      WHERE NOT EXISTS  
                            (SELECT *  
                               FROM SeqTbl S2  
                              WHERE S2.seq = S1.seq + 1)) END 
  FROM SeqTbl;
```

## HAVING句でサブクエリ -- 最頻値を求める

```
-- テーブル定義
CREATE TABLE Graduates
(name   VARCHAR(16) PRIMARY KEY,
 income INTEGER NOT NULL);

INSERT INTO Graduates VALUES('サンプソン', 400000);
INSERT INTO Graduates VALUES('マイク', 30000);
INSERT INTO Graduates VALUES('ホワイト', 20000);
INSERT INTO Graduates VALUES('アーノルド', 20000);
INSERT INTO Graduates VALUES('スミス', 20000);
INSERT INTO Graduates VALUES('ロレンス', 15000);
INSERT INTO Graduates VALUES('ハドソン', 15000);
INSERT INTO Graduates VALUES('ケント', 10000);
INSERT INTO Graduates VALUES('ベッカー', 10000);
INSERT INTO Graduates VALUES('スコット', 10000);
```

```
/*
最頻値を求めるSQL：その1 ALL述語の利用
収入が同じ卒業生をひとまとめにする集合を作り、その集合群から養素数が最も多い集合を探す
GROUP BY は、母集合を切り分けて部分集合を作る
*/
SELECT income, COUNT(*) AS cnt 
  FROM Graduates 
 GROUP BY income 
HAVING COUNT(*) >= ALL ( SELECT COUNT(*) 
                           FROM Graduates
                          GROUP BY income);
```

```
-- 最頻値を求めるSQL：その2：極値関数の利用
SELECT income, COUNT(*) AS cnt 
  FROM Graduates 
 GROUP BY income 
HAVING COUNT(*) >= ( SELECT MAX(cnt) 
                       FROM ( SELECT COUNT(*) AS cnt 
                                FROM Graduates 
                               GROUP BY income) TMP ) ;
```

## NULLを含まない集合を探す

- COUNT(*)とCOUNT(列名)の違い
  - パフォーマンス
  - COUNT(列名)は他の集約関数と同様、NULLを除外して考える。（COUNT(*)は全行数えるが、COUNT(列名)はそうでない。）

```
-- テーブル定義
CREATE TABLE NullTbl (col_1 INTEGER);

INSERT INTO NullTbl VALUES (NULL);
INSERT INTO NullTbl VALUES (NULL);
INSERT INTO NullTbl VALUES (NULL);
```

```
-- NULLを含む列に適用した場合、COUNT(*)とCOUNT(列名)の結果は異なる
SELECT COUNT(*), COUNT(col_1) 
  FROM NullTbl;
```

```
-- テーブル定義
CREATE TABLE Students
(student_id   INTEGER PRIMARY KEY,
 dpt          VARCHAR(16) NOT NULL,
 sbmt_date    DATE);

INSERT INTO Students VALUES(100, '理学部', '2018-10-10');
INSERT INTO Students VALUES(101, '理学部', '2018-09-22');
INSERT INTO Students VALUES(102, '文学部', NULL);
INSERT INTO Students VALUES(103, '文学部', '2018-09-10');
INSERT INTO Students VALUES(200, '文学部', '2018-09-22');
INSERT INTO Students VALUES(201, '工学部', NULL);
INSERT INTO Students VALUES(202, '経済学部', '2018-09-25');
```

```
-- 提出日にNULLを含まない学部を選択するクエリ：その1　COUNT関数の利用
SELECT dpt 
  FROM Students 
 GROUP BY dpt 
HAVING COUNT(*) = COUNT(sbmt_date);
```

```
-- 提出日にNULLを含まない学部を選択するクエリ：その2　CASE式の利用
SELECT dpt 
  FROM Students 
 GROUP BY dpt 
HAVING COUNT(*) = SUM(CASE WHEN sbmt_date IS NOT NULL 
                           THEN 1 ELSE 0 END);
```

### 特性関数の応用

```
-- テーブル定義
CREATE TABLE TestResults
(student_id CHAR(12) NOT NULL PRIMARY KEY,
 class   CHAR(1)  NOT NULL,
 sex     CHAR(1)  NOT NULL,
 score   INTEGER  NOT NULL);

INSERT INTO TestResults VALUES('001', 'A', '男', 100);
INSERT INTO TestResults VALUES('002', 'A', '女', 100);
INSERT INTO TestResults VALUES('003', 'A', '女', 49);
INSERT INTO TestResults VALUES('004', 'A', '男', 30);
INSERT INTO TestResults VALUES('005', 'B', '女', 100);
INSERT INTO TestResults VALUES('006', 'B', '男', 92);
INSERT INTO TestResults VALUES('007', 'B', '男', 80);
INSERT INTO TestResults VALUES('008', 'B', '男', 80);
INSERT INTO TestResults VALUES('009', 'B', '女', 10);
INSERT INTO TestResults VALUES('010', 'C', '男', 92);
INSERT INTO TestResults VALUES('011', 'C', '男', 80);
INSERT INTO TestResults VALUES('012', 'C', '女', 21);
INSERT INTO TestResults VALUES('013', 'D', '女', 100);
INSERT INTO TestResults VALUES('014', 'D', '女', 0);
INSERT INTO TestResults VALUES('015', 'D', '女', 0);
```

ToDo : この問題の続き
```
-- クラスの75%以上の生徒が80点以上のクラスを選択するクエリ
SELECT class 
  FROM TestResults 
 GROUP BY class 
HAVING COUNT(*) * 0.75 
        <= SUM(CASE WHEN score >= 80 
                    THEN 1 
                    ELSE 0 END) ;
```


SELECT class 
  FROM TestResults 
 GROUP BY class 
HAVING SUM(CASE WHEN score >= 50 AND sex = 'íj' 
                THEN 1 
                ELSE 0 END) 
         > SUM(CASE WHEN score >= 50 AND sex = 'èó' 
                    THEN 1 
                    ELSE 0 END) ; 

-- íjéqÇ∆èóéqÇÃïΩãœì_Çî‰ärÇ∑ÇÈÉNÉGÉäÇªÇÃ 1ÅFãÛèWçáÇ…ëŒÇ∑ÇÈïΩãœÇ0Ç≈ï‘Ç∑ 
SELECT class 
  FROM TestResults 
 GROUP BY class 
HAVING AVG(CASE WHEN sex = 'íj' THEN score ELSE 0 END) 
         < AVG(CASE WHEN sex = 'èó' THEN score ELSE 0 END) ;


-- íjéqÇ∆èóéqÇÃïΩãœì_Çî‰ärÇ∑ÇÈÉNÉGÉäÇªÇÃ 2ÅFãÛèWçáÇ…ëŒÇ∑ÇÈïΩãœÇ NULLÇ≈ï‘Ç∑ 
SELECT class 
  FROM TestResults 
 GROUP BY class 
HAVING AVG(CASE WHEN sex = 'íj' THEN score ELSE NULL END) 
        < AVG(CASE WHEN sex = 'èó' THEN score ELSE NULL END) ;




CREATE TABLE Teams
(member  CHAR(12) NOT NULL PRIMARY KEY,
 team_id INTEGER  NOT NULL,
 status  CHAR(8)  NOT NULL);

INSERT INTO Teams VALUES('ÉWÉáÅ[',   1, 'ë“ã@');
INSERT INTO Teams VALUES('ÉPÉì',     1, 'èoìÆíÜ');
INSERT INTO Teams VALUES('É~ÉbÉN',   1, 'ë“ã@');
INSERT INTO Teams VALUES('ÉJÉåÉì',   2, 'èoìÆíÜ');
INSERT INTO Teams VALUES('ÉLÅ[ÉX',   2, 'ãxâ…');
INSERT INTO Teams VALUES('ÉWÉÉÉì',   3, 'ë“ã@');
INSERT INTO Teams VALUES('ÉnÅ[Ég',   3, 'ë“ã@');
INSERT INTO Teams VALUES('ÉfÉBÉbÉN', 3, 'ë“ã@');
INSERT INTO Teams VALUES('ÉxÉX',     4, 'ë“ã@');
INSERT INTO Teams VALUES('ÉAÉåÉì',   5, 'èoìÆíÜ');
INSERT INTO Teams VALUES('ÉçÉoÅ[Ég', 5, 'ãxâ…');
INSERT INTO Teams VALUES('ÉPÅ[ÉKÉì', 5, 'ë“ã@');

-- ëSèÃï∂ÇèqåÍÇ≈ï\åªÇ∑ÇÈ 
SELECT team_id, member 
  FROM Teams T1 
 WHERE NOT EXISTS (SELECT * 
                     FROM Teams T2 
                    WHERE T1.team_id = T2.team_id 
                      AND status <> 'ë“ã@ ' ); 

/* ëSèÃï∂ÇèWçáÇ≈ï\åªÇ∑ÇÈÅFÇªÇÃ1 */
SELECT team_id
  FROM Teams
 GROUP BY team_id
HAVING COUNT(*) = SUM(CASE WHEN status = 'ë“ã@'
                           THEN 1
                           ELSE 0 END);

-- ëSèÃï∂ÇèWçáÇ≈ï\åªÇ∑ÇÈÅFÇªÇÃ 2 
SELECT team_id 
  FROM Teams 
 GROUP BY team_id 
HAVING MAX(status) = 'ë“ã@ ' 
   AND MIN(status) = 'ë“ã@ ';

-- ëçàıÉXÉ^ÉìÉoÉCÇ©Ç«Ç§Ç©ÇÉ`Å[ÉÄÇ≤Ç∆Ç…àÍóóï\é¶ 
SELECT team_id, 
       CASE WHEN MAX(status) = 'ë“ã@ ' AND MIN(status) = 'ë“ã@ ' 
            THEN 'ëçàıÉXÉ^ÉìÉoÉC ' 
            ELSE 'ë‡í∑ÅIÉÅÉìÉoÅ[Ç™ë´ÇËÇ‹ÇπÇÒ ' END AS status 
  FROM Teams GROUP BY team_id; 


-- àÍà”èWçáÇ∆ëΩèdèWçá
CREATE TABLE Materials
(center         CHAR(12) NOT NULL,
 receive_date   DATE     NOT NULL,
 material       CHAR(12) NOT NULL,
 PRIMARY KEY(center, receive_date));

INSERT INTO Materials VALUES('ìåãû'	,'2018-4-01',	'é‡');
INSERT INTO Materials VALUES('ìåãû'	,'2018-4-12',	'àüâî');
INSERT INTO Materials VALUES('ìåãû'	,'2018-5-17',	'ÉAÉãÉ~ÉjÉEÉÄ');
INSERT INTO Materials VALUES('ìåãû'	,'2018-5-20',	'àüâî');
INSERT INTO Materials VALUES('ëÂç„'	,'2018-4-20',	'ì∫');
INSERT INTO Materials VALUES('ëÂç„'	,'2018-4-22',	'ÉjÉbÉPÉã');
INSERT INTO Materials VALUES('ëÂç„'	,'2018-4-29',	'âî');
INSERT INTO Materials VALUES('ñºå√âÆ',	'2018-3-15',	'É`É^Éì');
INSERT INTO Materials VALUES('ñºå√âÆ',	'2018-4-01',	'íYëfç|');
INSERT INTO Materials VALUES('ñºå√âÆ',	'2018-4-24',	'íYëfç|');
INSERT INTO Materials VALUES('ñºå√âÆ',	'2018-5-02',	'É}ÉOÉlÉVÉEÉÄ');
INSERT INTO Materials VALUES('ñºå√âÆ',	'2018-5-10',	'É`É^Éì');
INSERT INTO Materials VALUES('ïüâ™'	,'2018-5-10',	'àüâî');
INSERT INTO Materials VALUES('ïüâ™'	,'2018-5-28',	'é‡');


-- éëçﬁÇÃÉ_ÉuÇ¡ÇƒÇ¢ÇÈãíì_ÇëIëÇ∑ÇÈ
SELECT center
  FROM Materials
 GROUP BY center
HAVING COUNT(material) <> COUNT(DISTINCT material);


SELECT center, CASE WHEN COUNT(material) <> COUNT(DISTINCT material)
                    THEN 'É_ÉuÇËóLÇË'
                    ELSE 'É_ÉuÇËñ≥Çµ'
                END AS status
  FROM Materials
 GROUP BY center;


-- É_ÉuÇËÇÃÇ†ÇÈèWçáÅFEXISTSÇÃóòóp
SELECT center, material
  FROM Materials M1
 WHERE EXISTS (SELECT *
                 FROM Materials M2
                WHERE M1.center = M2.center
                  AND M1.receive_date <> M2.receive_date
                  AND M1.material = M2.material);



/* ä÷åWèúéZÇ≈ÉoÉXÉPÉbÉgâêÕ */
CREATE TABLE Items
(item VARCHAR(16) PRIMARY KEY);
 
CREATE TABLE ShopItems
(shop VARCHAR(16),
 item VARCHAR(16),
    PRIMARY KEY(shop, item));

INSERT INTO Items VALUES('ÉrÅ[Éã');
INSERT INTO Items VALUES('éÜÉIÉÄÉc');
INSERT INTO Items VALUES('é©ì]é‘');

INSERT INTO ShopItems VALUES('êÂë‰',  'ÉrÅ[Éã');
INSERT INTO ShopItems VALUES('êÂë‰',  'éÜÉIÉÄÉc');
INSERT INTO ShopItems VALUES('êÂë‰',  'é©ì]é‘');
INSERT INTO ShopItems VALUES('êÂë‰',  'ÉJÅ[ÉeÉì');
INSERT INTO ShopItems VALUES('ìåãû',  'ÉrÅ[Éã');
INSERT INTO ShopItems VALUES('ìåãû',  'éÜÉIÉÄÉc');
INSERT INTO ShopItems VALUES('ìåãû',  'é©ì]é‘');
INSERT INTO ShopItems VALUES('ëÂç„',  'ÉeÉåÉr');
INSERT INTO ShopItems VALUES('ëÂç„',  'éÜÉIÉÄÉc');
INSERT INTO ShopItems VALUES('ëÂç„',  'é©ì]é‘');

-- ÉrÅ[ÉãÇ∆éÜÉIÉÄÉcÇ∆é©ì]é‘ÇÇ∑Ç◊ÇƒíuÇ¢ÇƒÇ¢ÇÈìXï‹ÇåüçıÇ∑ÇÈÅFä‘à·Ç¡ÇΩ SQL 
SELECT DISTINCT shop 
  FROM ShopItems 
 WHERE item IN (SELECT item FROM Items);



-- ÉrÅ[ÉãÇ∆éÜÉIÉÄÉcÇ∆é©ì]é‘ÇÇ∑Ç◊ÇƒíuÇ¢ÇƒÇ¢ÇÈìXï‹ÇåüçıÇ∑ÇÈÅFê≥ÇµÇ¢ SQL 
SELECT SI.shop 
  FROM ShopItems SI INNER JOIN Items I 
    ON SI.item = I.item 
 GROUP BY SI.shop 
HAVING COUNT(SI.item) = (SELECT COUNT(item) FROM Items);


-- COUNT(I.item)ÇÕÇ‡ÇÕÇ‚ 3Ç∆ÇÕå¿ÇÁÇ»Ç¢ 
SELECT SI.shop, COUNT(SI.item), COUNT(I.item) 
  FROM ShopItems SI INNER JOIN Items I 
    ON SI.item = I.item 
 GROUP BY SI.shop;


-- åµñßÇ»ä÷åWèúéZÅFäOïîåãçáÇ∆ COUNTä÷êîÇÃóòóp 
SELECT SI.shop 
  FROM ShopItems SI LEFT OUTER JOIN Items I 
    ON SI.item=I.item 
 GROUP BY SI.shop 
HAVING COUNT(SI.item) = (SELECT COUNT(item) FROM Items)  -- èåè 1 
   AND COUNT(I.item)  = (SELECT COUNT(item) FROM Items); -- èåè 2
```