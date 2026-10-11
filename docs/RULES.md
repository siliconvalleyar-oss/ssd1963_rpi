# Reglas del Proyecto (RULES)

## 1. Reglas de Oro (inviolables)
- **Nunca borrar archivos `.md`** (documentación).
- **Nunca borrar directorios o archivos de `skills/`**.
- **Nunca borrar scripts de `script_tools/*.sh`**.
- Cualquier cambio en estos elementos debe ser justificado y revisado en PR.

## 2. Flujo de trabajo con Git
- Rama principal: `main` (o `develop`).
- Crear ramas de feature desde `main`.
- Hacer `git pull` antes de empezar a trabajar.
- Los mensajes de commit deben ser claros y en presente.

## 3. Versión y Tags

### 3.1 Archivo VERSION
- `VERSION` contiene la versión actual (formato `vMAYOR.MENOR.PATCH`).
- **Antes de pushear un tag**: actualizar `VERSION` para que coincida con el tag.
- **Después de pushear un tag**: incrementar `VERSION` para la próxima iteración.
  - Ej: tras tag `v5.3.0` → `VERSION` = `v5.3.1` (patch) o `v5.4.0` (minor).

### 3.2 Flujo de versionado
```bash
# 1. Actualizar VERSION al tag deseado
echo "v5.3.0" > VERSION

# 2. Commit + tag + push
git add VERSION && git commit -m "v5.3.0"
git tag -a v5.3.0 -m "v5.3.0"
git push origin v5.3.0 --follow-tags

# 3. Incrementar VERSION para desarrollo
echo "v5.3.1" > VERSION
git add VERSION && git commit -m "bump v5.3.1"
git push
```

### 3.3 Compilación remota
Para compilar en el entorno remoto, **siempre** usar:
```bash
./script_tools/compilacion_remota.sh
