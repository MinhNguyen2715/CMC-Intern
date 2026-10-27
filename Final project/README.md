# Final Project

## 1. Test account
- Admin: 
    - Username: admin123
    - Password: admin123@
- Each {colour}:
    - Username: {x}colour
    - Password: {x}colour@123


## 2. Table Schemas

### 2.1 Table: `Users`

| Field Name | Data Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Mã định danh duy nhất của người dùng |
| `username` | `TEXT` | `UNIQUE NOT NULL` | Tên đăng nhập (Duy nhất trong hệ thống) |
| `password` | `TEXT` | `NOT NULL` | Mật khẩu tài khoản (Đã băm định dạng MD5) |

### SQL Statement:
```sql
CREATE TABLE IF NOT EXISTS Users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL
);
```
## 2.2 Table: Messages
| Field Name | Data Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
|`id`|`INTEGER`|`PRIMARY KEY AUTOINCREMENT`|Mã định danh duy nhất của tin nhắn|
|`sender`|`TEXT`|`NOT NULL`|Username của người gửi|
|`receiver`|`TEXT`|`NOT NULL`|Username của người nhận|
|`content`|`TEXT`|`NOT NULL`|Nội dung tin nhắn (Chuỗi UTF-8)|
|`timestamp`|`INTEGER`|`DEFAULT (strftime('%s', 'now'))`|`Thời gian gửi tin nhắn (Unix Timestamp)|

### SQL Statement:
```sql
CREATE TABLE IF NOT EXISTS Messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    content TEXT NOT NULL,
    timestamp INTEGER DEFAULT (strftime('%s', 'now'))
);
```
