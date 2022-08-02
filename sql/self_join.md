# 自己結合

## ポイント
- 自己結合は非等値結合と組み合わせて使うのが基本。
- GROUP BYと組み合わせると、再帰的集合を作ることができる。
- 本当に異なるテーブルを結合していると考えると理解しやすい。
- 物理ではなく論理の世界で考える。

## 重複・順列・組み合わせ

### テーブル定義
```
CREATE TABLE Products
(name VARCHAR(16) PRIMARY KEY,
 price INTEGER NOT NULL);

DELETE FROM Products;
INSERT INTO Products VALUES('りんご', 100);
INSERT INTO Products VALUES('みかん', 50);
INSERT INTO Products VALUES('バナナ', 80);
```

### 重複順列を得るSQL
```
SELECT P1.name AS name_1, P2.name AS name_2 
  FROM Products P1 CROSS JOIN Products P2;
```

### 同様のクエリ。
結合条件がある通常の結合を行うつもりが意図せぬクロス結合になってしまう恐れがあるので望ましくない。
```
SELECT P1.name AS name_1, P2.name AS name_2 
  FROM Products P1, Products P2;
```

### 順列を得るSQL
```
SELECT P1.name AS name_1, P2.name AS name_2 
  FROM Products P1 INNER JOIN Products P2 
    ON P1.name <> P2.name;
```

### 組み合わせを得るSQL
文字列型だと、辞書順比較となる。
```
SELECT P1.name AS name_1, P2.name AS name_2 
  FROM Products P1 INNER JOIN Products P2 
    ON P1.name > P2.name;
```

### 組み合わせを得るSQLを3列に拡張
```
SELECT P1.name AS name_1, 
       P2.name AS name_2, 
       P3.name AS name_3 
  FROM Products P1 
         INNER JOIN Products P2
          ON P1.name > P2.name 
            INNER JOIN Products P3 
              ON P2.name > P3.name;
```

## 重複行の削除

### テーブル定義
```
DROP TABLE Products;
CREATE TABLE Products
(name VARCHAR(16) NOT NULL,
 price INTEGER NOT NULL);

INSERT INTO Products VALUES('りんご', 50);
INSERT INTO Products VALUES('みかん', 100);
INSERT INTO Products VALUES('みかん', 100);
INSERT INTO Products VALUES('みかん', 100);
INSERT INTO Products VALUES('バナナ', 80);
```

### 重複行を削除するSQL：極地関数の使用
rowidは、OracleとPostgreSQLのみ対応している。
```
DELETE FROM Products P1 
 WHERE rowid < ( SELECT MAX(P2.rowid) 
                   FROM Products P2 
                  WHERE P1.name = P2. name 
                    AND P1.price = P2.price );
```

### 重複行を削除するSQL：非等値結合の使用
```
DELETE FROM Products P1 
 WHERE EXISTS ( SELECT * 
                  FROM Products P2 
                 WHERE P1.name = P2.name 
                   AND P1.price = P2.price 
                   AND P1.rowid < P2.rowid );
```

## 部分的に不一致なキーの検索

### テーブル定義
```
CREATE TABLE Addresses
(name VARCHAR(32),
 family_id INTEGER,
 address VARCHAR(32),
 PRIMARY KEY(name, family_id));

INSERT INTO Addresses VALUES('前田義明', '100', '東京都港区虎ノ門3-2-29');
INSERT INTO Addresses VALUES('前田由美', '100', '東京都港区虎ノ門3-2-92');
INSERT INTO Addresses VALUES('加藤茶', '200', '東京都新宿区西新宿2-8-1');
INSERT INTO Addresses VALUES('加藤勝', '200', '東京都新宿区西新宿2-8-1');
INSERT INTO Addresses VALUES('ホームズ', '300', 'ベーカー街221B');
INSERT INTO Addresses VALUES('ワトソン', '400', 'ベーカー街221B');
```

### 同じ家族だけど住所が違うレコード検索
```
SELECT DISTINCT A1.name, A1.address 
  FROM Addresses A1 INNER JOIN Addresses A2 
    ON A1.family_id = A2.family_id 
   AND A1.address <> A2.address ;
```

### テーブル定義
```
DELETE FROM Products;
INSERT INTO Products VALUES('りんご', 50);
INSERT INTO Products VALUES('みかん', 100);
INSERT INTO Products VALUES('ぶどう', 50);
INSERT INTO Products VALUES('スイカ', 80);
INSERT INTO Products VALUES('レモン', 30);
INSERT INTO Products VALUES('いちご', 100);
INSERT INTO Products VALUES('バナナ', 100);
```

### 値段が同じ商品の組み合わせを抽出する
```
SELECT DISTINCT P1.name, P1.price 
  FROM Products P1 INNER JOIN Products P2 
    ON P1.price = P2.price 
   AND P1.name <> P2.name
 ORDER BY P1.price;
```