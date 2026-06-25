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

## 3. Compilación remota
Para compilar en el entorno remoto, **siempre** usar:
```bash
./script_tools/compilacion_remota.sh
