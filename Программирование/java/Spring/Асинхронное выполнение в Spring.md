# Асинхронное выполнение в Spring Framework

## Введение
Асинхронное выполнение в Spring позволяет запускать методы в отдельных потоках, не блокируя основной поток выполнения. Это особенно полезно для тяжёлых операций, таких как сетевые вызовы или обработка данных, в веб-приложениях на базе Spring Boot. В этой статье мы рассмотрим аннотации `@EnableAsync` и `@Async`, конфигурацию, примеры кода и лучшие практики. Материал ориентирован на Java 21 и Spring Boot 3.5, включая интеграцию с виртуальными потоками.

## Основы асинхронного выполнения
Асинхронность улучшает производительность и отзывчивость приложений, позволяя обрабатывать несколько задач параллельно. Spring использует прокси для перехвата вызовов методов, отмеченных `@Async`, и делегирует их выполнение пулу потоков. Без асинхронности тяжёлые операции могут замедлить весь поток, например, в контроллерах Spring MVC.

Ключевые преимущества:
- **Не блокирует основной поток**: Идеально для веб-приложений.
- **Улучшает масштабируемость**: Позволяет обрабатывать больше запросов.
- **Поддержка виртуальных потоков**: В Java 21 можно использовать виртуальные потоки для лёгких задач.

Однако асинхронность добавляет сложность: управление потоками, обработка ошибок и тестирование.

## Конфигурация
Для включения асинхронности добавьте аннотацию `@EnableAsync` к основному классу приложения (с `@SpringBootApplication`) или конфигурационному классу.

```java
@SpringBootApplication
@EnableAsync
public class MyApplication {
    public static void main(String[] args) {
        SpringApplication.run(MyApplication.class, args);
    }
}
```

### Конфигурация TaskExecutor
По умолчанию Spring использует `SimpleAsyncTaskExecutor`, но для продакшена лучше настроить `ThreadPoolTaskExecutor`.

```java
@Configuration
public class AsyncConfig {

    @Bean(name = "taskExecutor")
    public Executor taskExecutor() {
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        executor.setCorePoolSize(4); // Минимальное количество потоков
        executor.setMaxPoolSize(10); // Максимальное количество потоков
        executor.setQueueCapacity(50); // Размер очереди
        executor.setThreadNamePrefix("Async-");
        executor.setWaitForTasksToCompleteOnShutdown(true); // Ожидать завершения задач при shutdown
        executor.initialize();
        return executor;
    }
}
```

В `@EnableAsync` укажите executor и обработчик исключений:
```java
@EnableAsync(executor = "taskExecutor", exceptionHandler = "asyncExceptionHandler")
```

### Альтернативный подход с AsyncConfigurerSupport
Для совместимости со старыми версиями Spring (например, 4.1+) можно наследоваться от `AsyncConfigurerSupport`:
```java
@Configuration
@EnableAsync
public class AsyncConfig extends AsyncConfigurerSupport {

    @Override
    public Executor getAsyncExecutor() {
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        executor.setCorePoolSize(4);
        executor.setMaxPoolSize(10);
        executor.setQueueCapacity(50);
        executor.setWaitForTasksToCompleteOnShutdown(true);
        executor.initialize();
        return executor;
    }

    @Override
    public AsyncUncaughtExceptionHandler getAsyncUncaughtExceptionHandler() {
        return new SimpleAsyncUncaughtExceptionHandler();
    }
}
```

## Аннотация @Async
Отмечает метод для асинхронного выполнения. Возвращаемый тип: `void`, `Future<T>` или `CompletableFuture<T>`.

```java
@Service
public class MyService {

    @Async
    public void asyncMethod() {
        // Тяжёлая операция
        System.out.println("Async method executed in thread: " + Thread.currentThread().getName());
    }

    @Async
    public Future<String> asyncMethodWithResult() {
        // Возвращает результат
        return new AsyncResult<>("Result");
    }

    @Async
    public CompletableFuture<String> asyncMethodCompletable() {
        return CompletableFuture.supplyAsync(() -> "Completable result");
    }
}
```

### Правила использования
- Метод должен быть `public`.
- Вызов из того же класса (self-invocation) не работает из-за прокси — Spring создаёт прокси вокруг бина, и внутренние вызовы обходят его. Альтернативы: `@EventListener` или рефакторинг (вынести в отдельный сервис).
- Не подходит для статических методов или конструкторов.

## Обработка результатов и ошибок
Для `void`-методов исключения не передаются вызывающему потоку по умолчанию — используйте `AsyncUncaughtExceptionHandler`.

```java
@Component
public class CustomAsyncExceptionHandler implements AsyncUncaughtExceptionHandler {

    @Override
    public void handleUncaughtException(Throwable ex, Method method, Object... params) {
        System.err.println("Async error in method " + method.getName() + ": " + ex.getMessage());
    }
}
```

Для методов с возвратом (`Future` или `CompletableFuture`) ошибки обрабатываются в вызывающем коде:
```java
CompletableFuture<String> future = service.asyncMethodCompletable();
future.thenAccept(result -> System.out.println(result))
      .exceptionally(ex -> {
          System.err.println("Error: " + ex.getMessage());
          return null;
      });
```

## Пример проекта
Рассмотрим полный пример с доменным классом `User`, сервисом и контроллером.

```java
// Доменный класс
public class User {
    private Long id;
    private String name;

    // Getters and setters
}

// Интерфейс сервиса
public interface UserDataService {
    CompletableFuture<User> getUserAsync(Long id);
    User getUserSync(Long id);
}

// Реализация сервиса
@Service
public class UserDataServiceImpl implements UserDataService {

    @Async
    @Override
    public CompletableFuture<User> getUserAsync(Long id) {
        // Симуляция тяжёлой операции
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return CompletableFuture.completedFuture(new User(id, "User " + id));
    }

    @Override
    public User getUserSync(Long id) {
        return new User(id, "User " + id);
    }
}

// Контроллер
@RestController
public class UserController {

    @Autowired
    private UserDataService userDataService;

    @GetMapping("/user/async/{id}")
    public CompletableFuture<ResponseEntity<User>> getUserAsync(@PathVariable Long id) {
        return userDataService.getUserAsync(id)
                .thenApply(user -> ResponseEntity.ok(user))
                .exceptionally(ex -> ResponseEntity.status(500).build());
    }

    @GetMapping("/user/sync/{id}")
    public ResponseEntity<User> getUserSync(@PathVariable Long id) {
        return ResponseEntity.ok(userDataService.getUserSync(id));
    }
}
```

## Лучшие практики
- **Выбирайте правильный TaskExecutor**: Для CPU-bound задач — виртуальные потоки; для I/O — ThreadPoolTaskExecutor.
- **Избегайте перегрузки**: Мониторьте пул потоков (метрики Spring Boot Actuator).
- **Тестирование**: Используйте `@SpringBootTest` с `@Async` для интеграционных тестов.
- **Альтернативы**: Для реактивного программирования рассмотрите WebFlux.
- **Graceful shutdown**: Настройте `waitForTasksToCompleteOnShutdown` для предотвращения потери данных.

## Интеграция с Java 21
В Java 21 используйте виртуальные потоки для лёгких асинхронных задач:
```java
@Bean
public Executor virtualThreadExecutor() {
    return Executors.newVirtualThreadPerTaskExecutor();
}
```
Укажите его в `@EnableAsync(executor = "virtualThreadExecutor")`. Виртуальные потоки эффективны для I/O-bound операций, но не для CPU-bound.

## Заключение
Асинхронное выполнение в Spring упрощает разработку отзывчивых приложений. С `@EnableAsync` и `@Async` вы можете легко интегрировать параллелизм, особенно с современными возможностями Java 21. Экспериментируйте с примерами, следуйте правилам и мониторьте производительность. Для дальнейших вопросов обращайтесь!