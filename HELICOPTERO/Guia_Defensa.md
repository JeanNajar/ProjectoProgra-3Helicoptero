# 🚁 GUÍA DE DEFENSA — HELICOPTER RESCUE

Guía completa con diagramas para explicar el proyecto ante el catedrático.
Todos los datos son exactos al código actual.

---

## 1. ARQUITECTURA GENERAL (el mapa del proyecto)

```
                        main.cpp
                            │
                            ▼
                     ┌───────────────┐
                     │     Menu      │  (QWidget) — ventana con 5 botones
                     └───────────────┘
                            │  jugar()
                            ▼
                     ┌───────────────┐
                     │     Game      │  (QGraphicsView) — la ventana del juego
                     └───────────────┘
                            │  tiene
        ┌───────────┬───────┼───────────┬───────────────┬──────────────┐
        ▼           ▼       ▼           ▼               ▼              ▼
   QGraphicsScene  MyHeli  Score      Health     ObstacleManager  LevelManager
   (el escenario)  (heli)  (puntaje)  (vida)     (obstáculos)     (cronómetro)
        │
        ├──► fondo (QGraphicsPixmapItem)
        ├──► ObstacleH (varios, creados por ObstacleManager)
        └──► FinishLine (zona verde, aparece al final)
```

**Flujo de arranque:**
1. `main.cpp` crea `Menu` y lo muestra
2. Al presionar "Jugar" → `game = new Game()` con `WA_DeleteOnClose`
3. `Game` crea la escena (800×600), el heli, el puntaje, la vida, el manager de obstáculos y el nivel
4. El nivel dura **30 segundos**, aparece un obstáculo **cada 3 segundos**

---

## 2. EL CICLO DE VIDA DE UN OBSTÁCULO (historia cronológica)

```
 1. NACIMIENTO                   2. VIDA                    3. MUERTE
 ─────────────                   ──────                     ────────
 ObstacleManager::               Cada 50 ms su              ObstacleH::move()
 spawnObstacle()                 timer llama move()         detecta:
    │                                │                        │
    ▼                                ▼                        ├─ choque con heli
 new ObstacleH(type)          setPos(x-5, y)  ← se          └─ salió de pantalla
    │                          mueve 5px a la izq.              (x+ancho < 0)
    ▼                                                           │
 new ObstacleH*[n+1]                                          ▼
 (matriz más grande)                              scene()->removeItem(this)
    │                                                          │
    ▼                                                          ▼
 copiar punteros viejos                          manager->notifyObstacleDied(this)
    │                                                          │
    ▼                                                          ├─ obstaculos[i] = nullptr (INMEDIATO)
 delete[] matriz vieja                                         └─ delete obstaculo (500 ms después,
    │                                                              para que suene el crash)
    ▼
 obstaculos = nuevaMatriz
 cantidad++
    │
    ▼
 scene->addItem(nuevo)
```

**Regla de memoria:** todo `new` tiene su `delete`, todo `new[]` tiene su `delete[]`.

---

## 3. EL DOBLE PUNTERO (la pregunta estrella)

### ¿Qué es?

```cpp
ObstacleH **obstaculos;   // ObstacleManager.h:30
```

```
obstaculos ──► [ptr0] [ptr1] [ptr2] [ptr3] ... [ptrN-1]     ← UNA fila (1D)
                 │      │      │      │            │
                 ▼      ▼      ▼      ▼            ▼
              ObstH  ObstH  ObstH  ObstH       ObstH        ← objetos reales
```

- `obstaculos` → guarda la **dirección del primer puntero** de la fila
- `obstaculos[i]` → **1ª desreferencia** → el puntero al obstáculo i
- `*obstaculos[i]` (o `->`) → **2ª desreferencia** → el objeto `ObstacleH`

### ¿Por qué doble y no simple?

| Con doble puntero (actual) | Con puntero simple |
|---|---|
| `obstaculos[i] = nullptr` (huecos) | ❌ no puedes poner un objeto en nullptr |
| `delete obstaculos[i]` (borrar uno) | ❌ no puedes borrar un elemento suelto |
| `obstaculos[i] = nuevo` (mover punteros) | ❌ copiarías objetos (Qt lo prohíbe) |

### ¿Cómo crece la matriz? (spawnObstacle)

```
ANTES:  obstaculos ──► [A] [B]            cantidad = 2

PASO 1: nuevaMatriz = new ObstacleH*[3]   (más grande)
PASO 2: nuevaMatriz = [A] [B] [ ]         (copiar punteros)
PASO 3: nuevaMatriz = [A] [B] [C]         (poner el nuevo al final)
PASO 4: delete[] obstaculos               (borrar fila vieja)
PASO 5: obstaculos = nuevaMatriz          (apuntar a la nueva)
PASO 6: cantidad++  →  3
```

### ¿Cómo muere un obstáculo? (notifyObstacleDied)

```
t = 0 ms    obstaculos[i] = nullptr   ← inmediato (evita puntero colgante)
t = 500 ms  delete obstaculo          ← retrasado (suena el crash)
```

La fila **no se encoge**: queda un hueco `nullptr`. Solo crece.
`removerObstacle()` existe (sí encoge) pero **nunca se llama**.
El destructor `~ObstacleManager` borra todo al cerrar el juego.

---

## 4. HERENCIA (9 clases)

```
QObject ────────────────► ObstacleManager, LevelManager
    │
    ├──► MyHeli     (junto con QGraphicsPixmapItem)   ← herencia MÚLTIPLE
    ├──► ObstacleH  (junto con QGraphicsPixmapItem)   ← herencia MÚLTIPLE
    └──► FinishLine (junto con QGraphicsRectItem)     ← herencia MÚLTIPLE

QGraphicsItem (base de todo lo dibujable)
    ├── QGraphicsPixmapItem ◄── MyHeli, ObstacleH
    ├── QGraphicsRectItem  ◄── FinishLine
    └── QGraphicsTextItem  ◄── Score, Health

QWidget ◄── Menu
QGraphicsView ◄── Game
```

**¿Qué les da cada padre?**
- `QObject` → señales/slots (`Q_OBJECT`), timers con parent, memoria automática
- `QGraphicsPixmapItem` → dibujarse en la escena, `setPixmap`, `setPos`, colisiones
- `QGraphicsView` → ser la ventana que muestra la escena (`setScene`, `fitInView`)
- `QWidget` → ser una ventana normal con botones

---

## 5. POLIMORFISMO (dónde está y dónde NO)

### ✅ Sí hay polimorfismo

1. **Punteros de clase base:** `scene->addItem(nuevo)` recibe un `QGraphicsItem*` pero le pasas un `ObstacleH*` — tratar un objeto derivado como su base.
2. **RTTI (identificar tipo real):** `collidingItems()` devuelve `QGraphicsItem*` y tú descubres el tipo real:
   ```cpp
   // ObstacleH.cpp:77 y 89
   if (typeid(*(colliding_items[i])) == typeid(MyHeli)) {
       MyHeli *heli = dynamic_cast<MyHeli*>(colliding_items[i]);
   ```
3. **Métodos virtuales sobreescritos:** `keyPressEvent`/`keyReleaseEvent` en MyHeli — Qt los llama a través del puntero base y ejecuta TU versión.

### ⚠️ NO hay polimorfismo entre obstáculos

- Todos los obstáculos son de la **misma clase** `ObstacleH`
- Los tipos (torre/cajón/tubería) se distinguen con un **enum** + `switch`, no con subclases
- Si quisieras polimorfismo real: crear `Torre`, `Cajon`, `Tuberia` que hereden de `ObstacleH` y sobreescriban `move()`

---

## 6. FÍSICAS (valores exactos de Physics.cpp)

| Constante | Valor | Significado |
|---|---|---|
| `gravedad` | 500 px/s² | empuja hacia abajo |
| `empuje` | −800 px/s² | negativo = hacia arriba |
| `velocidadMaxCaida` | 350 px/s | velocidad terminal |
| `velocidadMaxAterrizaje` | 200 px/s | aterrizaje seguro |
| `velocidadMaxHorizontal` | 300 px/s | tope lateral |
| `friccion` | 0.90 | factor por frame |

**Fórmulas:**
- Gravedad: `velY += 500 * dt` (luego clamp a 350)
- Empuje: `velY += -800 * dt` (clamp a −350)
- Fricción: `velX *= pow(0.90, dt*60)` — si `|velX| < 1` → 0 (no desliza infinito)
- Inclinación: `tilt = velY*0.06 + velX*0.02`, limitado a ±25°
- Aterrizaje seguro: `velY <= 200`

**Metáforas para explicarlo:**
- Gravedad = soltar una pelota
- Empuje = inflar un globo (sube)
- Fricción = patinar sobre hielo (frena de a poco)
- Inclinación = inclinarte en una bicicleta
- Aterrizaje brusco = huevo que se cae al piso

---

## 7. TIMERS (todos los relojes del juego)

| Timer | Intervalo | Qué dispara | Dónde |
|---|---|---|---|
| `physicsTimer` | 16 ms | `updatePhysics()` (físicas del heli) | MyHeli.cpp:32 |
| `rotorTimer` | 90 ms | `updateRotorAnimation()` (hélice) | MyHeli.cpp:37 |
| timer del obstáculo | 50 ms | `move()` (cada obstáculo tiene el suyo) | ObstacleH.cpp:45 |
| `finishCheckTimer` | 50 ms | `checkFinishLine()` (zona verde) | GAME.cpp:67 |
| `levelTimer` | 1000 ms | cuenta regresiva del nivel | LevelManager.cpp:13 |
| `spawnTimer` | 3000 ms | `spawnObstacles()` | LevelManager.cpp:26 |
| `singleShot` | 500 ms | `delete obstaculo` (tras morir) | ObstacleManager.cpp:158 |
| `singleShot` | 1000 ms | `delete this` (heli tras crash) | MyHeli.cpp:197 |

**Dato clave:** 40 ticks × 50 ms = **2 segundos** para ganar el nivel.

---

## 8. COLISIONES

```
ObstacleH::move()  (cada 50 ms)
        │
        ▼
collidingItems()  →  QList<QGraphicsItem*>  (todo lo que toca el obstáculo)
        │
        ▼
typeid(*item) == typeid(MyHeli) ?   ← RTTI: ¿es el helicóptero?
        │
        ├─ SÍ → health->decrease() (vida -1)
        │        ├─ suena CrashSound
        │        ├─ ¿health <= 0? → dynamic_cast → heli->crash()
        │        └─ removeItem + notifyObstacleDied (el obstáculo muere)
        │
        └─ NO → seguir moviéndose (setPos(x-5, y))
```

**Colisión con la zona verde (GAME.cpp:181-205):**
- Heli aterrizado: `y + alto >= scene->height() - 2`
- Sobre la zona: `colliding[i] == finishLine`
- Ambos durante 40 ticks (2 s) → **¡NIVEL COMPLETADO!**

---

## 9. SEÑALES Y SLOTS (cómo se comunican los objetos)

```
[Timer] timeout ──────► [Slot]
  physicsTimer ───────► MyHeli::updatePhysics
  rotorTimer ─────────► MyHeli::updateRotorAnimation
  timer (obstáculo) ──► ObstacleH::move
  finishCheckTimer ───► Game::checkFinishLine
  spawnTimer ─────────► Game::spawnObstacles
  levelTimer ─────────► lambda (cuenta regresiva)

[Botón] clicked ──────► [Slot del menú]
  btnJugar ───────────► Menu::jugar
  btnNivel ───────────► Menu::seleccionarNivel
  btnPuntajes ────────► Menu::puntajes
  btnAyuda ───────────► Menu::comoJugar
  btnSalir ───────────► Menu::salir
```

**¿Qué es una señal?** Un aviso que emite un objeto ("¡pasó algo!").
**¿Qué es un slot?** Una función que reacciona al aviso.
**¿Quién los conecta?** `connect(emisor, señal, receptor, slot)`.

---

## 10. RECURSOS (.qrc — la caja de materiales)

```
ResourcesForProject.qrc
├── /Sprites
│   ├── fondo_ciudad_cyberpunk_800x600.png   (fondo)
│   ├── helicoptero_frame1.png  (hélice 1/4)
│   ├── helicoptero_frame2.png  (hélice 2/4)
│   ├── helicoptero_frame3.png  (hélice 3/4)
│   ├── helicoptero_frame4.png  (hélice 4/4)
│   ├── torre_industrial_obstaculo_40x140.png (torre, escalada a 80×300)
│   ├── cajon_barrera_obstaculo_32x32.png     (cajón)
│   └── tuberias_techo_obstaculo_160x64.png   (tuberías del techo)
└── /Sounds
    ├── HelicopteroSound.mp3  (música, comentada en GAME.cpp)
    └── CrashSound.mp3        (sonido de choque)
```

Se acceden con `":/Sprites/..."` o `"qrc:/Sounds/..."`.

---

## 11. MEMORIA — QUIÉN BORRA QUÉ

| Objeto | Se crea con | Se borra con | Dónde |
|---|---|---|---|
| Obstáculos | `new ObstacleH` | `delete` (500 ms tras morir) | ObstacleManager |
| Matriz de obstáculos | `new ObstacleH*[n]` | `delete[]` | ObstacleManager (destructor) |
| `Physics` del heli | `new Physics` | `delete` | MyHeli (destructor) |
| `crashSound`/`crashAudio` (heli) | `new` | `delete` | MyHeli (destructor) |
| El heli mismo | `new MyHeli` | `delete this` (1 s tras crash) | MyHeli::crash |
| `obstacleManager` | `new` | `delete` | Game (destructor) |
| Items en la escena (fondo, score, health...) | `new` + `addItem` | **Qt** (la escena es dueña) | — |
| Timers con parent | `new QTimer(this)` | **Qt** (el parent los borra) | — |
| Widgets del menú | `new` + layout | **Qt** (el layout/parent) | — |
| `Game` | `new Game` | **Qt** (`WA_DeleteOnClose`) | Menu::jugar |

**Regla:** lo que creas SIN parent y SIN agregar a la escena → lo borras TÚ.

---

## 12. GUÍA RÁPIDA DE PREGUNTAS TÉCNICAS (Q&A)

### Punteros y memoria

**P: ¿Qué es un puntero?**
R: Una variable que guarda una dirección de memoria en vez de un valor. Con `*` accedes al valor guardado en esa dirección.

**P: ¿Qué es un doble puntero?**
R: Un puntero que apunta a otro puntero. `ObstacleH **obstaculos` apunta a una fila de punteros, y cada uno apunta a un `ObstacleH`.

**P: ¿Por qué usas doble puntero?**
R: Porque la fila guarda punteros, no objetos. Así puedo poner slots en nullptr, borrar obstáculos individuales y redimensionar moviendo solo punteros sin copiar objetos.

**P: ¿Cómo crece la matriz?**
R: En `spawnObstacle`: creo una fila más grande con `new[]`, copio los punteros viejos, agrego el nuevo al final, borro la fila vieja con `delete[]` y apunto a la nueva.

**P: ¿Qué pasa cuando un obstáculo muere?**
R: `notifyObstacleDied` pone su slot en nullptr (inmediato, evita puntero colgante) y programa el `delete` 500 ms después para que suene el crash.

**P: ¿Qué es un puntero colgante (dangling pointer)?**
R: Un puntero que apunta a memoria ya liberada. Lo evito poniendo nullptr después de borrar y verificando `scene() == nullptr` antes de usar objetos que pudieron morir.

**P: ¿Qué es un memory leak?**
R: Memoria que se reserva con `new` y nunca se libera. Lo evito con delete en el destructor y con los parents de Qt.

**P: ¿Por qué Game solo borra obstacleManager en su destructor?**
R: Porque el resto lo maneja Qt: la escena es dueña de los items, y los QObject con parent se borran solos.

**P: ¿Por qué el heli se borra con `delete this`?**
R: Porque nadie más tiene su puntero para borrarlo. Se retrasa 1 segundo para que suene el crash antes de morir.

### Herencia y polimorfismo

**P: ¿Dónde usas herencia?**
R: En 9 clases. Las gráficas heredan de QGraphicsItem (o derivadas) para dibujarse y colisionar; las que usan señales heredan de QObject. MyHeli, ObstacleH y FinishLine usan herencia múltiple (QObject + QGraphicsItem).

**P: ¿Dónde usas polimorfismo?**
R: Cuando la escena guarda todo como `QGraphicsItem*` y llama métodos virtuales; y cuando uso typeid/dynamic_cast para identificar el tipo real en colisiones.

**P: ¿Qué es RTTI?**
R: Identificación de tipo en tiempo de ejecución. `typeid` compara tipos, `dynamic_cast` baja un puntero base a derivado de forma segura.

**P: ¿Hay polimorfismo entre obstáculos?**
R: No. Todos son de la misma clase ObstacleH; los tipos se manejan con enum + switch.

### Físicas

**P: ¿Cómo funciona la gravedad?**
R: Cada 16 ms sumo `500 * dt` a velY, limitando la caída máxima a 350 px/s (velocidad terminal).

**P: ¿Cómo funciona el empuje?**
R: Al presionar arriba/espacio sumo `-800 * dt` a velY (negativo = sube), con límite de ascenso a −350.

**P: ¿Cómo funciona la fricción?**
R: Al soltar izquierda/derecha multiplico velX por `pow(0.90, dt*60)`; si baja de 1 px/s se redondea a 0 para que no deslice infinito.

**P: ¿Cómo se calcula la inclinación?**
R: `tilt = velY*0.06 + velX*0.02`, limitado a ±25°, y se suaviza con interpolación (0.15 por frame).

**P: ¿Cuándo es un aterrizaje seguro?**
R: Si velY ≤ 200 px/s. Si cae más rápido, `crash()` destruye el heli.

### Qt

**P: ¿Qué es una señal y un slot?**
R: Una señal es un aviso que emite un objeto; un slot es una función que reacciona. `connect` los une.

**P: ¿Qué timers usas?**
R: Físicas (16 ms), rotor (90 ms), movimiento de obstáculos (50 ms), chequeo de zona (50 ms), nivel (1 s), spawn (3 s), y dos singleShot para borrados con retraso.

**P: ¿Cómo detectas colisiones?**
R: `collidingItems()` devuelve todo lo que toca un item; comparo con typeid si es el heli.

**P: ¿Cómo se gana el nivel?**
R: Sobrevivir 30 s, esperar que la pantalla quede limpia, aterrizar en la zona verde y quedarse 2 segundos (40 ticks).

**P: ¿Qué es el .qrc?**
R: Un archivo que registra imágenes y sonidos como recursos del programa, accesibles con `":/prefijo/nombre"`.

**P: ¿Qué es WA_DeleteOnClose?**
R: Un atributo de Qt: cuando la ventana se cierra, Qt borra el objeto automáticamente. Así `Game` no queda en memoria.

---

## 13. FRASES CLAVE PARA LA DEFENSA (memoriza estas)

1. **Doble puntero:** "Es una fila dinámica de punteros; cada puntero apunta a un obstáculo. Es doble porque los elementos de la fila son punteros."
2. **Memoria:** "Todo lo que creamos con new lo liberamos con delete, y new[] con delete[]. Lo que tiene parent o está en la escena lo libera Qt."
3. **Puntero colgante:** "Ponemos nullptr después de borrar y verificamos scene() antes de usar un objeto que pudo morir."
4. **Herencia:** "Las clases gráficas heredan de QGraphicsItem para dibujarse; las que usan señales heredan de QObject; por eso hay herencia múltiple."
5. **Polimorfismo:** "La escena trata todo como QGraphicsItem* y usamos typeid/dynamic_cast para identificar el tipo real."
6. **Físicas:** "Gravedad 500, empuje −800, caída máxima 350, aterrizaje seguro ≤ 200, fricción 0.90."
7. **Delete con retraso:** "El delete va 500 ms después para que el sonido de crash termine de sonar."