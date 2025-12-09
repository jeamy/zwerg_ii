# Docker-Entwicklungsumgebung

## Voraussetzungen

- Docker
- Docker Compose

## Starten

```bash
# Container bauen und starten
docker-compose up --build

# Im Hintergrund starten
docker-compose up -d

# Logs anzeigen
docker-compose logs -f
```

## Zugriff

- **API**: http://localhost:8000
- **API-Docs**: http://localhost:8000/docs
- **Frontend**: http://localhost:8000/static/index.html
- **Health-Check**: http://localhost:8000/health

## Entwicklung

### Source-Code bleibt aktuell

Alle Änderungen an den Dateien werden sofort im Container sichtbar:
- `backend/` → `/app/backend/` (mit Hot-Reload)
- `frontend/` → `/app/frontend/`
- `database/` → `/app/database/`

### Container-Befehle

```bash
# Container stoppen
docker-compose stop

# Container neu starten
docker-compose restart

# Container löschen
docker-compose down

# Container mit Volumes löschen
docker-compose down -v

# In Container einloggen
docker exec -it zwergii-app bash

# Python-Shell im Container
docker exec -it zwergii-app python
```

## Protobuf kompilieren

```bash
# Im Container
docker exec -it zwergii-app bash
cd backend/proto
protoc --python_out=../app/services/proto/ *.proto
```

## Datenbank

Die SQLite-Datenbank wird in `database/dwarf.db` gespeichert und bleibt auch nach Container-Neustart erhalten.

## Troubleshooting

### Port bereits belegt

```bash
# Port 8000 freigeben
sudo lsof -ti:8000 | xargs kill -9
```

### Container neu bauen

```bash
docker-compose build --no-cache
docker-compose up
```

### Logs prüfen

```bash
docker-compose logs backend
```
