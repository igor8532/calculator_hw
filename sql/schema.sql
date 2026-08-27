CREATE TABLE IF NOT EXISTS operations (
    id SERIAL PRIMARY KEY,
    first_value INTEGER NOT NULL,
    second_value INTEGER NOT NULL,
    operation CHAR(1) NOT NULL,
    result INTEGER NOT NULL,
    status INTEGER NOT NULL
);
