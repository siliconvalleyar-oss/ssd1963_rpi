# Aprendizajes del Proyecto

## Git Push — Credenciales y Token

### Problema
El push falla con `403 Permission denied` cuando se usa HTTPS con el remote `siliconvalleyar-oss/pcb_cnc_rpi_prj.git`.

### Causa
El credential helper de macOS (`osxkeychain`) almacena credenciales para el usuario `git-user`, pero ese usuario no tiene permisos de escritura en el repositorio `siliconvalleyar-oss`.

### Solución — Token en config global

El token de GitHub (`ghp_...`) está configurado en la **config global de git**:

```bash
# Verificar token (NO compartir estos datos)
git config --global --list | grep -i "user.password"
```

Salida:
```
user.password=ghp_XXXXXXX
user.name=siliconvalleyar-oss
user.email=siliconvalleyar@gmail.com
```

### Proceso de Push

```bash
# 1. Temporalmente configurar remote con token
git remote set-url origin https://siliconvalleyar-oss:<TOKEN>@github.com/siliconvalleyar-oss/pcb_cnc_rpi_prj.git

# 2. Push
git push origin kicad_v10 --tags

# 3. Limpiar remote (quitar token de la URL)
git remote set-url origin https://github.com/siliconvalleyar-oss/pcb_cnc_rpi_prj.git
```

**IMPORTANTE:** Siempre limpiar la URL después del push para no exponer el token.

### Verificación de credenciales

```bash
# Verificar remote actual
git remote -v

# Verificar usuario git
git config user.name
git config user.email

# Verificar acceso al remote
git ls-remote origin

# Verificar estado
git status
git log --oneline -5
```

## macOS Keychain

Las credenciales de GitHub se almacenan en:
```bash
security find-internet-password -s "github.com" -a "git-user"
```

Pero estas son para el usuario `git-user` (solo lectura en `siliconvalleyar-oss`).

## Version Tagging

```bash
# Ver último tag
git tag -l "v*" --sort=-version:refname | head -1

# Crear tag
git tag -a v1.0.X -m "descripcion"

# Push con tags
git push origin kicad_v10 --tags
```

## Git Config Global

Ubicación: `~/.config/git/config` o `~/.gitconfig`

```bash
# Ver configuración completa
git config --global --list

# Editar
git config --global --edit
```

## Licencias

### KiCad
- KiCad 10.0.4 usa **GPL v2+** (embebido en el binario)
- No incluye archivo LICENSE en el bundle macOS
- El archivo LICENSE del proyecto debe crearse manualmente

### Ubicación de archivos de licencia
- KiCad bundle: `/Applications/KiCad/KiCad.app/Contents/Resources/Licenses/`
- Solo contiene: `Python/LICENSE.txt` (PSF License)
- No hay licencia KiCad como archivo separado

### Proyecto
- `.gitignore` menciona `LICENSE` pero no existe
- Crear con `touch LICENSE` y agregar texto GPL v2+

## Errores Comunes

### 403 Permission denied
- Causa: usuario sin permisos de escritura
- Solución: usar token con permisos de push

### SSH Permission denied
- Causa: SSH key no configurada en la cuenta
- Solución: agregar `~/.ssh/id_ed25519.pub` en GitHub → Settings → SSH Keys

### KiCad parse error
- Causa: `knockout` no válido en PCB text
- Solución: eliminar `knockout` de los archivos .kicad_pcb

## Cómo buscar licencias y credenciales en esta PC

### Credenciales de Git (GitHub)

El credential helper de git está configurado como `store`, lo que significa
que las credenciales se guardan en texto plano en un archivo local.

**Ubicación del archivo de credenciales:**
```
~/.git-credentials
```

**Formato del archivo** (cada línea es un repo):
```
https://USUARIO:TOKEN@github.com
```

**Cómo verificar sin exponer la clave:**
```bash
# Verificar que el archivo existe (sin mostrar contenido)
ls -la ~/.git-credentials

# Verificar que el credential helper está activo
git config --global credential.helper
# Salida esperada: store

# Verificar que las credenciales funcionan (autentica sin mostrar token)
git ls-remote origin 2>&1 | head -3

# Verificar usuario configurado
git config user.name
git config user.email
```

**Si el archivo no existe o las credenciales fallan:**
```bash
# Crear el archivo (el formato es una URL por línea)
echo "https://USUARIO:TOKEN_GITHUB@github.com" > ~/.git-credentials
chmod 600 ~/.git-credentials

# O configurar via git config
git config --global credential.helper store
```

**Ubicación alternativa (configuración global):**
```
~/.gitconfig
```
Contiene `user.name`, `user.email` y `credential.helper`.

### Archivos de licencia del proyecto

**KiCad:**
- No incluye archivo LICENSE separado en el bundle
- Licencia: GPL v2+ (embebida en el binario)
- En Linux: `/usr/share/kicad/licenses/` (si existe)

**Ubicación de este proyecto:**
```
docs/LEARNINGS.md    — este archivo
.gitignore           — menciona LICENSE pero puede no existir
```

### Variables de entorno útiles

```bash
# Home del usuario
echo $HOME          # /home/optimus

# Config de git
echo $GIT_CONFIG_GLOBAL  # ~/.gitconfig por defecto

# Temp directory (para archivos temporales de FreeRouting)
echo $TMPDIR        # /tmp o /var/tmp
```

### Resumen de ubicaciones importantes

| Archivo | Ubicación | Contenido |
|---------|-----------|-----------|
| Credenciales git | `~/.git-credentials` | Token GitHub en URL |
| Config git global | `~/.gitconfig` | user, email, credential.helper |
| Config KiCad | `~/.config/kicad/10.0/` | pcbnew.json, kicad.json |
| Plugin Freerouting | `~/.local/share/kicad/10.0/3rdparty/plugins/app_freerouting_kicad-plugin/` | plugin.py, jar/ |
| FreeRouting JAR | `proyecto/freerouting-2.2.4.jar` | Binario (gitignored) |
| Script autoroute | `proyecto/scripts/run_autoroute.sh` | Wrapper con filtrado de capas |

## Git / Versionado

**Todo push debe llevar su tag.** No se pushea sin tag. El tag identifica la versión.

### Flujo de versionado

1. Obtener el último tag publicado (ej: `v1.0.0`).
2. El archivo `VERSION` debe coincidir con ese tag (sin `v`).
3. Calcular la siguiente versión: `tag + 0.0.1` (ej: `v1.0.0` → `1.0.1`).
4. Actualizar `VERSION` a la nueva versión.
5. Confirmar cambios y pushear con el nuevo tag.

### Reglas

- **Los mensajes de commit deben seguir conventional commits**: `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`, `test:`.
- **No eliminar tags publicados.** Si hay error, crear nuevo tag.
- **El tag y `VERSION` siempre deben coincidir** (tag con `v`, `VERSION` sin `v`).
- **VERSION siempre refleja el último tag.** El archivo `VERSION` en la raíz del proyecto debe contener exactamente el número del último tag publicado (sin prefijo `v`). Ejemplo: si el último tag es `v1.0.5`, el archivo `VERSION` contiene `1.0.5`.
- **Cada commit significativo debe tener su tag.** No se salta ningún número de versión. Si se salta un número, se pierde la secuencia.
- **Tag = VERSION.** Cada vez que se hace push de un tag, el archivo `VERSION` debe actualizarse al mismo número (sin `v`). La relación es: `git tag v1.0.5` → `VERSION` = `1.0.5`.
- **El ciclo patch 0-9 es obligatorio.** No se puede pasar de `v1.0.9` a `v1.1.1`. Debe ir a `v1.1.0`. Esto asegura que cada minor tenga exactamente 10 patches.
- **No se puede retroceder de versión.** Una vez publicado un tag, no se puede reemplazar. Si hay un error, se crea un nuevo tag con el siguiente número en la secuencia.
- **El archivo VERSION empieza en 1.0.0** que corresponde al tag `v1.0.0`.

### ¿Para qué sirve este esquema?

- Cada versión es única e inmutable
- Se puede saber exactamente cuántas versiones han existido
- No hay ambigüedad sobre qué versión sigue
- Compatible con semver estricto

### Primeros 100 tags

| # | Tag | # | Tag | # | Tag | # | Tag | # | Tag |
|---|------|---|------|---|------|---|------|---|------|
| 1 | v1.0.0 | 21 | v1.2.0 | 41 | v1.4.0 | 61 | v1.6.0 | 81 | v1.8.0 |
| 2 | v1.0.1 | 22 | v1.2.1 | 42 | v1.4.1 | 62 | v1.6.1 | 82 | v1.8.1 |
| 3 | v1.0.2 | 23 | v1.2.2 | 43 | v1.4.2 | 63 | v1.6.2 | 83 | v1.8.2 |
| 4 | v1.0.3 | 24 | v1.2.3 | 44 | v1.4.3 | 64 | v1.6.3 | 84 | v1.8.3 |
| 5 | v1.0.4 | 25 | v1.2.4 | 45 | v1.4.4 | 65 | v1.6.4 | 85 | v1.8.4 |
| 6 | v1.0.5 | 26 | v1.2.5 | 46 | v1.4.5 | 66 | v1.6.5 | 86 | v1.8.5 |
| 7 | v1.0.6 | 27 | v1.2.6 | 47 | v1.4.6 | 67 | v1.6.6 | 87 | v1.8.6 |
| 8 | v1.0.7 | 28 | v1.2.7 | 48 | v1.4.7 | 68 | v1.6.7 | 88 | v1.8.7 |
| 9 | v1.0.8 | 29 | v1.2.8 | 49 | v1.4.8 | 69 | v1.6.8 | 89 | v1.8.8 |
| 10 | v1.0.9 | 30 | v1.2.9 | 50 | v1.4.9 | 70 | v1.6.9 | 90 | v1.8.9 |
| 11 | v1.1.0 | 31 | v1.3.0 | 51 | v1.5.0 | 71 | v1.7.0 | 91 | v1.9.0 |
| 12 | v1.1.1 | 32 | v1.3.1 | 52 | v1.5.1 | 72 | v1.7.1 | 92 | v1.9.1 |
| 13 | v1.1.2 | 33 | v1.3.2 | 53 | v1.5.2 | 73 | v1.7.2 | 93 | v1.9.2 |
| 14 | v1.1.3 | 34 | v1.3.3 | 54 | v1.5.3 | 74 | v1.7.3 | 94 | v1.9.3 |
| 15 | v1.1.4 | 35 | v1.3.4 | 55 | v1.5.4 | 75 | v1.7.4 | 95 | v1.9.4 |
| 16 | v1.1.5 | 36 | v1.3.5 | 56 | v1.5.5 | 76 | v1.7.5 | 96 | v1.9.5 |
| 17 | v1.1.6 | 37 | v1.3.6 | 57 | v1.5.6 | 77 | v1.7.6 | 97 | v1.9.6 |
| 18 | v1.1.7 | 38 | v1.3.7 | 58 | v1.5.7 | 78 | v1.7.7 | 98 | v1.9.7 |
| 19 | v1.1.8 | 39 | v1.3.8 | 59 | v1.5.8 | 79 | v1.7.8 | 99 | v1.9.8 |
| 20 | v1.1.9 | 40 | v1.3.9 | 60 | v1.5.9 | 80 | v1.7.9 | 100 | v1.9.9 |

## Edge.Cuts — Cómo dibujar un PCB con esquinas curvas

### Fórmula general

Para una placa de **W x H mm** con esquinas de radio **R mm**:

1. **Centro del tablero:** `(cx, cy)`
2. **Esquina superior-izquierda del cuadrado:** `(cx - W/2, cy - H/2)`
3. **Cada esquina del arco:** el vértice original movido **R mm hacia adentro** del tablero
4. **Radio del arco:** R mm
5. **Líneas rectas:** se acortan R mm en cada extremo para dejar espacio al arco

### Coordenadas para 99x99mm, R=5mm

Centro del tablero: `(117, 119)` → Esquinas del cuadrado: `(67.5, 69.5)` a `(166.5, 168.5)`

| Elemento | Inicio | Fin | Centro del arco |
|----------|--------|-----|-----------------|
| Línea top | (72.5, 69.5) | (161.5, 69.5) | - |
| Arco TR | (161.5, 69.5) | (166.5, 74.5) | (161.5, 74.5) |
| Línea right | (166.5, 74.5) | (166.5, 163.5) | - |
| Arco BR | (166.5, 163.5) | (161.5, 168.5) | (161.5, 163.5) |
| Línea bottom | (161.5, 168.5) | (72.5, 168.5) | - |
| Arco BL | (72.5, 168.5) | (67.5, 163.5) | (72.5, 163.5) |
| Línea left | (67.5, 163.5) | (67.5, 74.5) | - |
| Arco TL | (67.5, 74.5) | (72.5, 69.5) | (72.5, 74.5) |

### Puntos medios de los arcos (para KiCad)

Fórmula: `mid = center + R * (cos(angle), sin(angle))`

| Arco | Ángulo | Punto medio |
|------|--------|-------------|
| TR | 315° | (165.036, 70.964) |
| BR | 45° | (165.036, 167.036) |
| BL | 135° | (68.964, 167.036) |
| TL | 225° | (68.964, 70.964) |

### Python para calcular

```python
import math

W, H, R = 99, 99, 5
cx, cy = 117, 119  # centro del tablero

x1, y1 = cx - W/2, cy - H/2  # esquina sup-izq
x2, y2 = cx + W/2, cy + H/2  # esquina inf-der

# Cada centro de arco = vértice movido R mm hacia adentro
centers = {
    'TR': (x2 - R, y1 + R),  # (161.5, 74.5)
    'BR': (x2 - R, y2 - R),  # (161.5, 163.5)
    'BL': (x1 + R, y2 - R),  # (72.5, 163.5)
    'TL': (x1 + R, y1 + R),  # (72.5, 74.5)
}

# Ángulos del punto medio de cada arco
angles = {'TR': 315, 'BR': 45, 'BL': 135, 'TL': 225}

for name, (cx_, cy_) in centers.items():
    a = math.radians(angles[name])
    mx = round(cx_ + R * math.cos(a), 3)
    my = round(cy_ + R * math.sin(a), 3)
    print(f'{name}: center=({cx_},{cy_}) mid=({mx},{my})')
```

### Stroke
- Width: 0.001mm
- Type: default
- Layer: Edge.Cuts
