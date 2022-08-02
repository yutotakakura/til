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

```
CREATE TABLE Students
(name CHAR(16) PRIMARY KEY,
 age  INTEGER);

INSERT INTO Students VALUES('ÉuÉâÉEÉì', 22);
INSERT INTO Students VALUES('ÉâÉäÅ[', 19);
INSERT INTO Students VALUES('ÉWÉáÉì', NULL);
INSERT INTO Students VALUES('É{ÉMÅ[', 21);
```

```
SELECT *
  FROM Students
 WHERE age IS DISTINCT FROM 20;
```

```
SELECT 'abc' || '' AS string FROM dual;
```

```
SELECT 'abc' || NULL AS string FROM dual;
```

```
CREATE TABLE EmptyStr
( str CHAR(8),
  description CHAR(16));

INSERT INTO EmptyStr VALUES('', 'empty string');
INSERT INTO EmptyStr VALUES(NULL, 'NULL' );
```

```
SELECT 'abc' || str AS string, description
  FROM EmptyStr;
```