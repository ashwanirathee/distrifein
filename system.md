#### Current Mind Model:

```
+-------------------------+
|        Client UI        |
|  (Web App / Mobile App) |
+-----------+-------------+
            |
            ▼
+---------------------------+
|     API Gateway / LB      |  ← Auth, Rate Limit, Route
+-----------+---------------+
            |
+-----------▼---------------------------------------------------------------+
|                                Backend Services                           |
|----------------------------------------------------------------------------|
|  Auth Service     |  Posts Service   |  Comments Service  |  Feed Service  |
|-------------------|------------------|---------------------|----------------|
| User Reg/Login    | Create/Fetch     | Add/Fetch           | Personalized   |
| JWT Tokens        | Upvotes/Downvotes| Votes & Nesting     | or Global Feed |
+----------------------------------------------------------------------------+
            |
            ▼
+------------------------------+      +-----------------------------+
|     Distributed Database     |◄────►|    Caching Layer (Redis)    |
| (e.g., Cassandra / CouchDB)  |      | For hot feeds & votes cache |
+------------------------------+      +-----------------------------+
            |
            ▼
+----------------------------+
|  Distributed File Storage  | ← Optional: for media/images (e.g., IPFS, S3)
+----------------------------+
```