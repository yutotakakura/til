# 目的
- なんとなく使っているSQLの構造を正しく理解すること。
- 大規模データを扱う際にも、スロークエリにならないクエリ設計を行えるようになること。

# 環境構築メモ

## MySQLの環境構築
Dockerコンテナの立ち上げ。プライベートマシンが M1 Mac なので、プラットフォームを指定する。

```
docker run --platform linux/x86_64 -p 3306:3306 -e MYSQL_ROOT_PASSWORD=password -d mysql:latest
```

コンテナに入り、以下コマンドでMySQLに入れることを確認。
```
mysql -h127.0.0.1 -uroot -ppassword -P3306
```

DBeaverで接続時に'Public Key Retrieval is not allowed'のエラーが出たら、JDBCドライバのプロパティを変更する。
- 'allowPublicKeyRetrieval' : 'true'
- 'useSSL' : 'false'

# 参考文献
- [達人に学ぶSQL徹底指南書 第2版 初級者で終わりたくないあなたへ](https://www.amazon.co.jp/-/en/%E3%83%9F%E3%83%83%E3%82%AF/dp/4798157821/ref=pd_lpo_1?pd_rd_i=4798157821&psc=1)
- [M1搭載MacのDockerでMySQLを動かしてみる](https://gihyo.jp/dev/serial/01/mysql-road-construction-news/0167)
- [SQL Error [08001]: Public Key Retrieval is not allowedの対処法 [JDBC:MySQL]](https://qiita.com/noy__/items/f53e9e6210e02233d414)

# 使用ツール
- [Docker](https://www.docker.com/)
- [DBeaver](https://dbeaver.io/)