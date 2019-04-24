import * as React from 'react';
import * as ReactDOM from 'react-dom';
import './index.css';
import registerServiceWorker from './registerServiceWorker';
import { createStore } from 'redux';
import { enthusiasm } from './reducers/index';
import { StoreState } from './types/index';

// import Hello from './containers/Hello';
import RGBSelector from './components/rgb';
import { Provider } from 'react-redux';

const store = createStore<StoreState,any,any,any>(enthusiasm, {
  enthusiasmLevel: 1,
  languageName: 'TypeScript',
});

// ReactDOM.render(
//   <Provider store={store}>
//     <Hello />
//   </Provider>,
//   document.getElementById('root') as HTMLElement
// );

ReactDOM.render(
  <Provider store={store}>
    <RGBSelector title="R"/>
  </Provider>,
  document.getElementById('root') as HTMLElement
);

registerServiceWorker();
