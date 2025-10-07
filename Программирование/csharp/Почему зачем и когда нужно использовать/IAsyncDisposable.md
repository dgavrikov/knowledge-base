# Почему, зачем и когда нужно использовать IAsyncDisposable

В .NET Framework 1.0 появился интерфейс `IDisposable` для освобождения неуправляемых ресурсов. С ростом асинхронного программирования в .NET Core и C# 8.0 ввели `IAsyncDisposable` — интерфейс для асинхронного освобождения ресурсов. Это позволяет избежать блокировки потоков при освобождении, например, сетевых соединений или файлов. В этой статье разберем, зачем и когда его использовать.

## IDisposable и его ограничения

`IDisposable` — ключевой интерфейс для детерминированного освобождения ресурсов. Метод `Dispose()` вызывается в блоке `using` и очищает память, закрывает файлы или соединения. Но если освобождение требует асинхронных операций (например, flush буфера или отправка финального пакета), `Dispose()` блокирует поток, что нежелательно в асинхронных приложениях.

Пример синхронного `Dispose`:

```csharp
public class MyResource : IDisposable
{
    public void Dispose()
    {
        // Синхронное освобождение
        _stream.Close();
    }
}
```

В асинхронных сценариях это может вызвать deadlock или замедлить приложение.

## IAsyncDisposable для асинхронного освобождения

`IAsyncDisposable` появился в .NET Core 3.0 и .NET Standard 2.1. Он имеет метод `DisposeAsync()`, возвращающий `ValueTask`, что позволяет асинхронно освобождать ресурсы без блокировки.

Пример:

```csharp
public class MyAsyncResource : IAsyncDisposable
{
    private Stream _stream;

    public async ValueTask DisposeAsync()
    {
        await _stream.FlushAsync();
        _stream.Close();
    }
}
```

Это идеально для ресурсов, где освобождение включает I/O-операции.

## Паттерны использования IAsyncDisposable

В C# 8.0 добавлен `await using` для автоматического вызова `DisposeAsync()`. Аналогично `using`, но с `await`.

Примеры:

```csharp
// Простое использование
await using (var resource = new MyAsyncResource())
{
    // Работа с ресурсом
} // DisposeAsync() вызывается автоматически

// Без using — ручной вызов
var resource = new MyAsyncResource();
try
{
    // Работа
}
finally
{
    await resource.DisposeAsync();
}
```

Классы вроде `Stream` или `HttpClient` в .NET Core реализуют `IAsyncDisposable`. Для совместимости с `IDisposable` можно реализовать оба интерфейса.

## Когда использовать IAsyncDisposable?

Используйте `IAsyncDisposable`, если:
- Освобождение ресурсов требует асинхронных операций (I/O, сеть).
- Вы работаете в асинхронных методах и хотите избежать блокировки.
- Ресурс часто используется в асинхронном коде.

Для простых ресурсов с синхронным освобождением достаточно `IDisposable`. В библиотеках .NET (например, EF Core) `IAsyncDisposable` применяется для асинхронного закрытия соединений с БД.

## Миграция с IDisposable на IAsyncDisposable

Если у вас есть класс с `IDisposable`, и вы хотите добавить асинхронное освобождение:
1. Добавьте `IAsyncDisposable` к интерфейсам.
2. Реализуйте `DisposeAsync()` с асинхронной логикой.
3. Оставьте `Dispose()` для совместимости, но пусть он вызывает `DisposeAsync().GetAwaiter().GetResult()` (с осторожностью, чтобы избежать deadlock).
4. В коде клиентов замените `using` на `await using`.

Пример миграции:

```csharp
public class MyResource : IDisposable, IAsyncDisposable
{
    public void Dispose()
    {
        DisposeAsync().GetAwaiter().GetResult();
    }

    public async ValueTask DisposeAsync()
    {
        await _stream.FlushAsync();
        _stream.Close();
    }
}
```

## Бенчмарки производительности

В асинхронных приложениях `IAsyncDisposable` предотвращает блокировки, улучшая отзывчивость. Бенчмарки показывают, что `await using` с `IAsyncDisposable` может снизить время ожидания на 20-50% в I/O-зависимых операциях по сравнению с синхронным `Dispose()`, особенно при высокой нагрузке.

## Что дальше с IAsyncDisposable?

В будущих версиях .NET `IAsyncDisposable` станет ещё распространеннее. Например, в .NET 6 добавили его в `System.Text.Json` для асинхронного освобождения сериализаторов.

**Ссылки:**
- [Документация Microsoft по IAsyncDisposable](https://docs.microsoft.com/en-us/dotnet/api/system.iasyncdisposable)
- [Async/Await в C#](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/concepts/async/)

**Теги:** c#, .net, асинхронное программирование, управление ресурсами  
**Хабы:** Программирование, .NET, C#