# 3倫理値とNULL

## ポイント
- NULLは値ではない
- 値ではないので、述語もまともに適用できない
- 無理やり適用するとunknownが生じる
- unknownが論理演算に紛れ込むと、SQLが直観に反する動作をする。
- これに対処するには、段階的なステップを分けてSQLの動作を追うことが有効。

## 理論

### 3倫理値の真理表(NOT)
| x   | NOT x |
| --- | ----- |
| t   | f     |
| u   | u     |
| f   | t     | 

### 3倫理値の真理表(AND)
| AND | t   | u   | f   |
| --- | --- | --- | --- | 
| t   | t   | u   | f   |
| u   | u   | u   | f   | 
| f   | f   | f   | f   | 

### 3倫理値の真理表(OR)
| OR  | t   | u   | f   |
| --- | --- | --- | --- | 
| t   | t   | t   | t   |
| u   | t   | u   | u   | 
| f   | t   | u   | f   | 

## 実践

### 排中律が成立しない
命題とその否定をまたはでつなげてできる命題は全て真である命題を、2倫理値で排中律という。<br>
例）ジョンは20歳から、20歳でないか、どちらかである。<br>
テーブル定義
```
CREATE TABLE Students
(name CHAR(16) PRIMARY KEY,
 age  INTEGER);

INSERT INTO Students VALUES('ブラウン', 22);
INSERT INTO Students VALUES('ラリー', 19);
INSERT INTO Students VALUES('ジョン', NULL);
INSERT INTO Students VALUES('ボギー', 21);
```
年齢が20歳か、20歳でない生徒を選択する<br>
以下のクエリでは、ジョンは抽出されない
```
SELECT * FROM Students
WHERE age = 20 OR age <> 20;
```
その理由を、ジョンのパターンを段階的に分解して考える。<br>
ジョンは年齢がNULLであるから
```
SELECT * FROM Students
WHERE NULL = 20 OR NULL <> 20;
```
比較述語にNULLを適用するとunknownになる
```
SELECT * FROM Students
WHERE unknown OR unknown;
```
unknown OR unknown は unknown になる（理論のマトリクスを参照）
```
SELECT * FROM Students
WHERE unknown;
```
SQLで選択結果に含まれるのは、trueに評価される行だけなので、ジョンは抽出されなかった。<br>
ジョンを結果に含めるには、第三の条件を追加する必要がある。
```
SELECT * FROM Students
WHERE age = 20 OR age <> 20 OR age IS NULL;
```

### CASE式とNULL
col_1が1なら「◯」を、NULLなら「×」を返すCASE式？
```
CASE col_1
    WHEN 1 THEN '◯'
    WHEN NULL THEN '×'
END
```
この式は、絶対に×を返さない。<br>
なぜなら、二つ目のWHERE句は col_1 = NULL の省略形であり、unknownが返るから。<br>
正しくは、以下。
```
CASE WHEN col_1 = 1 THEN '◯'
     WHEN col_1 IS NULL THEN '×'
END
```

### NOT INとNOT EXISTSは同値ではない
テーブル定義
```
CREATE TABLE class_A
(name CHAR(16) PRIMARY KEY,
 age  INTEGER,
 city CHAR(16)
);

INSERT INTO class_A VALUES('ブラウン', 22, '東京');
INSERT INTO class_A VALUES('ラリー', 19, '埼玉');
INSERT INTO class_A VALUES('ボギー', 21, '千葉');
```

```
CREATE TABLE class_B
(name CHAR(16) PRIMARY KEY,
 age  INTEGER,
 city CHAR(16)
);

INSERT INTO class_B VALUES('斎藤', 22, '東京');
INSERT INTO class_B VALUES('田丸', 23, '東京');
INSERT INTO class_B VALUES('山田', NULL, '東京');
INSERT INTO class_B VALUES('和泉', 18, '千葉');
INSERT INTO class_B VALUES('武田', 20, '千葉');
INSERT INTO class_B VALUES('石川', 19, '神奈川');
```

Bクラスの東京在住の生徒と年齢が一致しないAクラスの生徒を選択するSQL?<br>
結果は一行も選択されない
```
SELECT * FROM class_A 
WHERE age NOT IN 
	(SELECT age FROM class_B
	WHERE city = '東京')
```
段階的に見ていく。<br>
サブクエリを実行して、年齢のリストを取得
```
SELECT * FROM class_A 
WHERE age NOT IN (22, 23, NULL);
```
NOT INとINを使って同値変換
```
SELECT * FROM class_A
WHERE NOT age IN (22, 23, NULL);
```
IN述語をORで同値変換
```
SELECT * FROM class_A
WHERE NOT ((age = 22) OR (age = 23) OR (age = NULL));
```
ド・モルガンの法則を使って同値変換
```
SELECT * FROM class_A
WHERE NOT ((age = 22) AND NOT (age = 23) AND NOT (age = NULL));
```
NOTと=を<>で同値変換
```
SELECT * FROM class_A
WHERE (age <> 22) AND (age <> 23) AND (age <> NULL));
```
NULLに<>を適用するとunknownになる
```
SELECT * FROM class_A
WHERE (age <> 22) AND (age <> 23) AND unknown);
```
AND演算にunknownが含まれていると、結果がtrueにならない
```
SELECT * FROM class_A
WHERE false または unknown;
```
正しいSQL：ラリーとポギーが選択される
```
SELECT * FROM class_A ca 
WHERE NOT EXISTS 
	(SELECT * FROM class_B cb
	WHERE ca.age = cb.age
	AND cb.city = '東京');
```
段階的に追う
サブクエリにおいて、NULLとの比較を行う
```
SELECT * FROM class_A ca 
WHERE NOT EXISTS 
	(SELECT * FROM class_B cb
	WHERE ca.age = NULL
	AND cb.city = '東京');
```
NULLに = を適用すると unknown になる
```
SELECT * FROM class_A ca 
WHERE NOT EXISTS 
	(SELECT * FROM class_B cb
	WHERE unknown
	AND cb.city = '東京');
```
AND演算にunknownが含まれていると、結果がtrueにならない
```
SELECT * FROM class_A ca 
WHERE NOT EXISTS 
	(SELECT * FROM class_B cb
	WHERE false または unknown;
```
サブクエリが結果を返さないので、反対にNOT EXISTS は true になる
EXISTS句は、trueとfalseしか返さない
```
SELECT * FROM class_A ca
WHERE true;
```

### 限定述語とNULL
ALLとANY。ANYはINと同義なのであまり使われないので、ALLを見ていく。<br>
ALLは、比較述語と併用して、「〜全てと等しい」や「〜全てよりも大きい」という意味を表す。<br>
一度山田に年齢をセット。
```
UPDATE class_B SET age = 20 WHERE name = '山田';
```
Bクラスの東京在住の誰よりも若いAクラスの生徒を選択する
```
SELECT * FROM class_A 
	WHERE age < ALL (SELECT age FROM class_B 
						WHERE city = '東京');
```
また山田の年齢をNULLにする。
```
UPDATE class_B SET age = NULL WHERE name = '山田';
```
この状態では、上記のSQLは一件も選択されない。段階的に見ていく。
```
SELECT * FROM class_A
    WHERE age < ALL (22, 23, NULL);
```
ALL述語をANDで同値変換
```
SELECT * FROM class_A
    WHERE (age < 22) AND (age < 23) AND (age < NULL);
```
NULLに<を適用すると、unknownになる。
```
SELECT * FROM class_A
    WHERE (age < 22) AND (age < 23) AND unknown;
```
AND演算にunknownが含まれていると、結果がtrueにならない。
```
SELECT * FROM class_A
    WHERE false または unknown;
```

### 限定述語と極値関数は、同値ではない
先ほどのSQLを極値関数で書き換える。<br>
Bクラスの東京在住の誰よりも若いAクラスの生徒を選択する
```
SELECT * FROM class_A 
	WHERE age < ALL (SELECT MIN(age) FROM class_B 
						WHERE city = '東京');
```
結果として、山田の年齢がNULLでも、ポギーとラリーが返る。<br>
これは、極値関数が集計の際にNULLを排除するから。<br>
極値関数を使えば、NULLの場合にも対応できるように思えるが、意味の違いがある。<br>
<br>
ALL述語：彼は東京在住の誰よりも若い
極値関数：彼は東京在住の最も若い生徒よりも若い
<br>
現実世界では、意味は同じだが、SQLでは述語の入力が空集合だったときに異なる挙動をする。（一件も返らない。）<br>
東京在住の生徒をclass_Bから削除する。
```
DELETE FROM class_B WHERE city = '東京';
```
極値関数がNULLを返す。
```
SELECT * FROM class_A 
	WHERE age < NULL;
```
NULLLに < を適用すると unknownになる。
```
SELECT * FROM class_A 
	WHERE unknown;
```

### 集約関数とNULL
東京在住の生徒の平均年齢よりも若い生徒を選択するSQL?　→ 一件も返らない。
```
SELECT * FROM class_A 
	WHERE age < (SELECT AVG(age) FROM class_B 
						WHERE city = '東京');
```