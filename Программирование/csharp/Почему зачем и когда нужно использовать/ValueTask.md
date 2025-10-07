# Почему, зачем и когда нужно использовать ValueTask

В .NET Framework 4 появилось пространство `System.Threading.Tasks`, а с ним и класс `Task`. Этот тип и порожденный от него `Task<TResult>` стали стандартами асинхронного программирования в .NET, особенно с операторами `async`/`await` в C# 5. Однако для улучшения производительности в случаях с частыми синхронными завершениями в .NET Core 2.0 ввели `ValueTask<TResult>` — структуру, которая минимизирует выделение памяти. В этой статье разберем, зачем и когда её использовать.

## Task и его ограничения

`Task` играет ключевую роль в асинхронном программировании: он представляет "обещание" завершения операции. Можно ожидать его с `await`, использовать в коллекциях или комбинировать с другими задачами. Но гибкость имеет цену — `Task` это класс, что приводит к выделению памяти и нагрузке на GC, даже при синхронном завершении.

Примеры оптимизаций: кеширование `Task.CompletedTask` для void-методов или булевых значений. Библиотеки вроде `MemoryStream` тоже кешируют результаты, но для большинства случаев объект `Task<TResult>` всё равно создаётся заново.

## ValueTask<TResult> для синхронного выполнения

`ValueTask<TResult>` — структура, которая может обернуть как `TResult`, так и `Task<TResult>`. Это позволяет возвращать результат без выделения памяти в куче, если операция завершается синхронно.

Пример:

```csharp
public override ValueTask<int> ReadAsync(byte[] buffer, int offset, int count)
{
    try
    {
        int bytesRead = Read(buffer, offset, count);
        return new ValueTask<int>(bytesRead);
    }
    catch (Exception e)
    {
        return new ValueTask<int>(Task.FromException<int>(e));
    }
}
```

Так работают новые перегрузки `Stream.ReadAsync` в .NET Core 2.1.

## ValueTask<TResult> для асинхронного выполнения

В .NET Core 2.1 `ValueTask<TResult>` поддерживает `IValueTaskSource<TResult>` для повторного использования объектов из пула, минимизируя выделения даже при асинхронном выполнении. Примеры: `Socket.ReceiveAsync` и `Socket.SendAsync`.

Интерфейс `IValueTaskSource<TResult>`:

```csharp
public interface IValueTaskSource<out TResult>
{
    ValueTaskSourceStatus GetStatus(short token);
    void OnCompleted(Action<object> continuation, object state, short token, ValueTaskSourceOnCompletedFlags flags);
    TResult GetResult(short token);
}
```

Большинству разработчиков не нужно реализовывать его вручную — используйте готовые классы вроде `ManualResetValueTaskSourceCore<TResult>` в .NET Core 3.0.

## Необобщённый ValueTask

`ValueTask` (без обобщения) появился в .NET Core 2.1 для методов без возвращаемого значения, аналогично `Task`. Он тоже поддерживает оптимизации через `IValueTaskSource`.

## Паттерны применения ValueTask

`ValueTask`/`ValueTask<TResult>` подходят для простого `await`, но имеют ограничения из-за повторного использования объектов. Не повторяйте `await`, не ожидайте параллельно и не используйте `.GetAwaiter().GetResult()` без проверки.

Правильные паттерны:

```csharp
// GOOD
int result = await SomeValueTaskReturningMethodAsync();

// GOOD
Task<int> t = SomeValueTaskReturningMethodAsync().AsTask();
```

Неправильные:

```csharp
// BAD: повторный await
ValueTask<int> vt = SomeValueTaskReturningMethodAsync();
int result = await vt;
int result2 = await vt;
```

Метод `.Preserve()` возвращает `ValueTask`/`ValueTask<TResult>`, пригодный для повторного или параллельного ожидания:

```csharp
// Пример с Preserve()
ValueTask<int> vt = SomeValueTaskReturningMethodAsync();
ValueTask<int> preservedVt = vt.Preserve(); // Теперь можно await несколько раз
int result1 = await preservedVt;
int result2 = await preservedVt; // Без проблем
```

Продвинутый паттерн: проверка `IsCompletedSuccessfully` перед `await` для оптимизации.

## Когда использовать ValueTask?

По умолчанию выбирайте `Task`/`Task<TResult>` — они проще. `ValueTask`/`ValueTask<TResult>` подходят, когда:
- Метод вызывается только с `await`.
- Выделение памяти критично.
- Синхронное выполнение часто, или есть эффективное повторное использование при асинхронном.

Для абстрактных методов учитывайте эти условия в реализациях.

## Миграция с Task на ValueTask

Если у вас есть метод, возвращающий `Task`, и вы хотите оптимизировать его для частых синхронных завершений:
1. Замените `Task<TResult>` на `ValueTask<TResult>`.
2. Проверьте все места вызова: убедитесь, что используется только `await`, без кеширования или параллели.
3. Если нужно сохранить гибкость `Task`, используйте `.AsTask()`.

Пример миграции:

```csharp
// Было
public Task<int> GetResultAsync() => Task.FromResult(42);

// Стало
public ValueTask<int> GetResultAsync() => new ValueTask<int>(42);
```

## Бенчмарки производительности

В бенчмарках .NET Core 2.1 `ValueTask` может снизить аллокации на 50-80% в сценариях с частым синхронным завершением по сравнению с `Task`. Например, в высоконагруженных API с чтением из буфера это экономит память и уменьшает давление на GC.

## Что дальше с ValueTask?

В системных библиотеках .NET продолжат добавлять методы с `ValueTask`, например `IAsyncEnumerator<T>.MoveNextAsync` в .NET Core 3.0 возвращает `ValueTask<bool>` для оптимизации асинхронных итераторов.

**Ссылки:**
- [Документация Microsoft по ValueTask](https://docs.microsoft.com/en-us/dotnet/api/system.threading.tasks.valuetask-1)
- [Async/Await в C#](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/concepts/async/)
