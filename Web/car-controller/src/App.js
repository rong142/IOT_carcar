
import './App.css';
import { Card } from 'primereact/card';
import React, { useState, useEffect, useRef } from 'react';
import { classNames } from 'primereact/utils';
import { Button } from 'primereact/button';
import { useEventListener } from 'primereact/hooks';

function App() {
  const [pressed, setPressed] = useState(false);
  const [value, setValue] = useState('');
  const [direction, setDirection] = useState('');

  const [image, setImage] = useState('');

  const pressed_up = pressed && direction === 'up';
  const pressed_left = pressed && direction === 'left';
  const pressed_down = pressed && direction === 'down';
  const pressed_right = pressed && direction === 'right';
  const pressed_pause = pressed && direction === 'pause';

  const ws_image_connected = useRef(false);
  const ws_controller_connected = useRef(false);

  const first = useRef(false);

  const ws_image = useRef(null);
  const ws_controller = useRef(null);

  const onKeyDown = (e) => {
    setPressed(true);

    if (e.code === 'Space') {
      setValue('space');

      return;
    }

    setValue(e.key);
  };

  const [bindKeyDown, unbindKeyDown] = useEventListener({
    type: 'keydown',
    listener: (e) => {
      onKeyDown(e);
      // console.log(e.code);
      switch (e.code) {
        case 'ArrowUp':
          setDirection('up');
          break;
        case 'ArrowDown':
          setDirection('down');
          break;
        case 'ArrowLeft':
          setDirection('left');
          break;
        case 'ArrowRight':
          setDirection('right');
          break;
        case 'KeyW':
          setDirection('up');
          break;
        case 'KeyS':
          setDirection('down');
          break;
        case 'KeyA':
          setDirection('left');
          break;
        case 'KeyD':
          setDirection('right');
          break;
        case 'Space':
          setDirection('pause');
          break;
        default:
          setDirection('');
          break;
      }
    }
  });

  const [bindKeyUp, unbindKeyUp] = useEventListener({
    type: 'keyup',
    listener: (e) => {
      setPressed(false);
    }
  });

  useEffect(() => {
    bindKeyDown();
    bindKeyUp();

    return () => {
      unbindKeyDown();
      unbindKeyUp();
    };
  }, [bindKeyDown, bindKeyUp, unbindKeyDown, unbindKeyUp]);

  useEffect(() => {
    if (ws_controller_connected.current && direction) {
      ws_controller.current.send(direction);
    } else {
      console.log('ws not connected yet');
    }
  }, [direction]);

  useEffect(() => {
    if (!first.current) {
      console.log('init');
      first.current = true;

      ws_controller.current = new WebSocket('ws://192.168.3.4:9080');
      ws_controller.current.onopen = () => {
        console.log('open connection');
        ws_controller_connected.current = true;
      }
      ws_controller.current.onclose = () => {
        console.log('close connection');
        ws_controller_connected.current = false;
      }

      ws_image.current = new WebSocket('ws://192.168.3.4:9090');
      ws_image.current.onopen = () => {
        console.log('open connection');
        ws_image_connected.current = true;
      }
      ws_image.current.onclose = () => {
        console.log('close connection');
        ws_image_connected.current = false;
      }
      ws_image.current.onmessage = event => {
        const data = event.data;
        setImage(data);

      }
    }
  }, []);

  return (
    <div className="card">
      <Card title="畫面顯示" className='homecard'>
        {ws_image_connected.current ? <img className='image' src={`data:image/jpeg;base64,${image}`} alt='' /> : <p>Not connected</p>}
      </Card>

      <Card title="汽車控制器" className='homecard'>
        <Button
          icon='pi pi-arrow-up'
          className={classNames('button border-1 surface-border border-round-md py-3 px-4 font-semibold text-lg transition-all transition-duration-150', { 'shadow-1': pressed_up, 'shadow-5': !pressed_up })}
          style={{
            transform: pressed_up ? 'translateY(5px)' : 'translateY(0)'
          }}
          onClick={() => setDirection('up')}
        />
        <br />
        <Button
          icon='pi pi-arrow-left'
          className={classNames('button border-1 surface-border border-round-md py-3 px-4 font-semibold text-lg transition-all transition-duration-150', { 'shadow-1': pressed_left, 'shadow-5': !pressed_left })}
          style={{
            transform: pressed_left ? 'translateY(5px)' : 'translateY(0)'
          }}
          onClick={() => setDirection('left')}
        />
        <Button
          icon='pi pi-arrow-down'
          className={classNames('button border-1 surface-border border-round-md py-3 px-4 font-semibold text-lg transition-all transition-duration-150', { 'shadow-1': pressed_down, 'shadow-5': !pressed_down })}
          style={{
            transform: pressed_down ? 'translateY(5px)' : 'translateY(0)'
          }}
          onClick={() => setDirection('down')}
        />
        <Button
          icon='pi pi-arrow-right'
          className={classNames('button border-1 surface-border border-round-md py-3 px-4 font-semibold text-lg transition-all transition-duration-150', { 'shadow-1': pressed_right, 'shadow-5': !pressed_right })}
          style={{
            transform: pressed_right ? 'translateY(5px)' : 'translateY(0)'
          }}
          onClick={() => setDirection('right')}
        />
        <br />
        <Button
          label='暫停'
          className={classNames('button border-1 surface-border border-round-md py-3 px-4 font-semibold text-lg transition-all transition-duration-150', { 'shadow-1': pressed_pause, 'shadow-5': !pressed_pause })}
          style={{
            width: '150px',
            transform: pressed_pause ? 'translateY(5px)' : 'translateY(0)'
          }}
          onClick={() => setDirection('pause')}
        />
      </Card>
    </div>
  );
}

export default App;
