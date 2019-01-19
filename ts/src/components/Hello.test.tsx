import * as React from 'react';
import * as enzyme from 'enzyme';
import * as sinon from 'sinon';
import Hello from './Hello';

it('renders the correct text when no enthusiasm level is given', () => {
  const hello = enzyme.shallow(<Hello name='Tom' />);
  expect(hello.find(".greeting").text()).toEqual('Hello Tom!')
});

it('renders the correct text with an explicit enthusiasm of 1', () => {
  const hello = enzyme.shallow(<Hello name='Tom' enthusiasmLevel={1} />);
  expect(hello.find(".greeting").text()).toEqual('Hello Tom!')
});

it('renders the correct text with an explicit enthusiasm of 5', () => {
  const hello = enzyme.shallow(<Hello name='Tom' enthusiasmLevel={5} />);
  expect(hello.find(".greeting").text()).toEqual('Hello Tom!!!!!');
});

it('throws when the enthusiasm level is 0', () => {
  expect(() => {
    enzyme.shallow(<Hello name='Tom' enthusiasmLevel={0} />)
  }).toThrow();
});

it('throws when the enthusiasm level is negative', () => {
  expect(() => {
    enzyme.shallow(<Hello name='Tom' enthusiasmLevel={-1} />)
  }).toThrow();
});

it('registers onIncrement at click of + button', () => {
  const spy = sinon.spy();
  const hello = enzyme.shallow(<Hello name='Tom' enthusiasmLevel={3} onIncrement={spy} />);
  hello.find(".increment").simulate('click');
  expect(spy.calledOnce).toBe(true);
}

// it('registers onDecrement at click of - button', () => {
//   const spy = sinon.spy();
//   const hello = enzyme.shallow(<Hello name='Tom' enthusiasmLevel={3} onDecrement={spy} />);
//   hello.find(".decrement").simulate('click');
//   expect(spy.calledOnce).toBe(true);
// }
