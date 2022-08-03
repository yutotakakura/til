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
| t   | f   | u   | f   |
| u   | u   | u   | f   | 
| f   | t   | f   | f   | 

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